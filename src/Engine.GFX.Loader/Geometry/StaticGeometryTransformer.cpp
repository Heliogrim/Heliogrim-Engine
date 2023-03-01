#include "StaticGeometryTransformer.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX/Buffer/VirtualBufferPage.hpp>
#include <Engine.GFX/Buffer/VirtualBufferView.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>
#include <Engine.GFX/Memory/AllocationResult.hpp>
#include <Engine.GFX/Memory/AllocatedMemory.hpp>
#include <Engine.GFX/Memory/VkAllocator.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

static smr<StaticGeometryResource> loadWithAssimp(
    const non_owning_rptr<const engine::assets::StaticGeometry> request_,
    mref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    cref<StaticGeometryTransformer::request_type::options> options_
);

StaticGeometryTransformer::StaticGeometryTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

StaticGeometryTransformer::response_type::type StaticGeometryTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    auto asset = static_cast<non_owning_rptr<const assets::Asset>>(request_);
    auto src = next_(_STD move(asset), next_type::next_request_type::options {});

    /**/

    return loadWithAssimp(_STD move(request_), _STD move(src), _pool, _STD move(options_));
}

/* Assimp Implementation */

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static smr<StaticGeometryResource> loadWithAssimp(
    const non_owning_rptr<const engine::assets::StaticGeometry> request_,
    mref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<pool::GlobalResourcePool> pool_,
    cref<StaticGeometryTransformer::request_type::options> options_
) {

    /**/

    Assimp::Importer importer {};
    const u32 ppFlags = { aiProcess_PreTransformVertices };

    /**/

    const streamsize size = src_->size();
    if (size < 0) {
        IM_CORE_WARNF(
            R"(Tried to load an asset `{}` with assimp without a associated source file size limit)",
            request_->getAssetName()
        );
        return {};
    }

    Vector<_::byte> buffer {};
    buffer.resize(static_cast<u64>(size));

    streamsize bytes;
    src_->get(0, size, buffer.data(), bytes);
    assert(bytes >= size);

    // Attention: Temporary workaround, cause assimp can not fetch splited files while using load from memory
    const char* pHint = "";
    if (request_->sources().front().path().ends_with(R"(.obj)")) {
        pHint = "obj";
    }

    auto* const scene = importer.ReadFileFromMemory(buffer.data(), size, ppFlags, pHint);

    if (scene == nullptr) {
        IM_CORE_ERRORF(
            "Could not load asset `{}` with assimp due to the following error:\n{}",
            request_->getAssetName(),
            importer.GetErrorString()
        );
        return {};
    }

    /**/

    Vector<u32> indices {};
    Vector<vertex> vertices {};

    /**/

    {

        auto** meshes = scene->mMeshes;
        const auto meshCount = scene->mNumMeshes;

        for (auto meshIdx = 0; meshIdx < meshCount; ++meshIdx) {

            auto* mesh = meshes[meshIdx];
            const auto fc = mesh->mNumFaces;
            const auto vc = mesh->mNumVertices;

            /**/

            indices.reserve(indices.size() + fc * 3ui32);
            const auto idxOff = indices.size();

            for (u32 j = 0; j < fc; ++j) {

                const auto& face = mesh->mFaces[j];
                for (u8 k = 0; k < face.mNumIndices; ++k) {
                    indices.push_back(idxOff + face.mIndices[k]);
                }
            }

            /**/

            vertices.reserve(vertices.size() + vc);

            auto* const vcc = mesh->mColors[0];
            auto* const vtc = mesh->mTextureCoords[0];
            auto* const vn = mesh->mNormals;
            auto* const vta = mesh->mTangents;

            using Vertex = vertex;

            constexpr Vertex::color_type white = Vertex::color_type { 255ui8 };

            for (u32 j = 0; j < vc; ++j) {

                const auto& vertex = mesh->mVertices[j];

                vertices.push_back(
                    Vertex {
                        { vertex.x, vertex.y, vertex.z },
                        vcc ?
                            Vertex::color_type {
                                static_cast<u8>(vcc[j].r * 255.0F),
                                static_cast<u8>(vcc[j].g * 255.0F),
                                static_cast<u8>(vcc[j].b * 255.0F)
                            } :
                            white,
                        vtc ? Vertex::uvm_type { vtc[j].x, 1.0F - vtc[j].y, vtc[j].z } : math::vec3_zero,
                        vn ? Vertex::normal_type { vn[j].x, vn[j].y, vn[j].z } : math::vec3_up,
                        vta ? Vertex::tangent_type { vta[j].x, vta[j].y, vta[j].z } : math::vec3_zero
                    }
                );

            }

        }

    }

    /**/

    uptr<VirtualBufferView> indexBuffer {};
    uptr<VirtualBufferView> vertexBuffer {};

    /**/

    {
        const auto indexSize = sizeof(u32) * indices.size();
        indexBuffer = pool_->allocateIndexBuffer({ { indexSize } });

        const auto* const firstPage = indexBuffer->pages().front();
        const auto alignment = firstPage->memory()->allocated()->layout.align;

        const auto required = (indexSize / alignment) + ((indexSize % alignment) ? 1ui64 : 0ui64);

        /**/

        for (u32 page = 0ui32; page < required; ++page) {

            const auto pageSize = _STD min(indexSize - (page * alignment), alignment);

            constexpr auto shift = 7ui64;
            constexpr auto mask = 0b0111'1111ui64;

            const auto aligned = ((pageSize >> shift) << shift) + ((pageSize & mask) ? + 1ui64 << shift : 0ui64);

            auto* const memory = indexBuffer->pages()[page]->memory()->allocated();
            const auto patchSize = _STD min(aligned, memory->size);

            /**/

            memory->map(patchSize);
            memory->write(
                static_cast<const ptr<const _::byte>>(static_cast<ptr<void>>(indices.data())) + (page * alignment),
                patchSize
            );
            memory->flush(patchSize);
            memory->unmap();
        }
    }

    /**/

    {
        const auto vertexSize = sizeof(vertex) * vertices.size();
        vertexBuffer = pool_->allocateVertexBuffer({ { vertexSize } });

        const auto* const firstPage = vertexBuffer->pages().front();
        const auto alignment = firstPage->memory()->allocated()->layout.align;

        const auto required = (vertexSize / alignment) + ((vertexSize % alignment) ? 1ui64 : 0ui64);

        /**/

        for (u32 page = 0ui32; page < required; ++page) {

            const auto pageSize = _STD min(vertexSize - (page * alignment), alignment);

            constexpr auto shift = 7ui64;
            constexpr auto mask = 0b0111'1111ui64;

            const auto aligned = ((pageSize >> shift) << shift) + ((pageSize & mask) ? + 1ui64 << shift : 0ui64);

            auto* const memory = vertexBuffer->pages()[page]->memory()->allocated();
            const auto patchSize = _STD min(aligned, memory->size);

            /**/

            memory->map(patchSize);
            memory->write(
                static_cast<const ptr<const _::byte>>(static_cast<ptr<void>>(vertices.data())) + (page * alignment),
                patchSize
            );
            memory->flush(patchSize);
            memory->unmap();
        }
    }

    /**/

    using derived_type = ::hg::engine::resource::UniqueResource<StaticGeometryResource::value_type>;
    auto dst = make_smr<StaticGeometryResource, derived_type>(
        new derived_type(_STD move(indexBuffer), _STD move(vertexBuffer))
    );
    dst->setAssociation(request_);

    return dst;
}

/**/
