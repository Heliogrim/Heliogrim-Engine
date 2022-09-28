#include "StaticGeometryLoader.hpp"

#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Engine.Common/Math/Coordinates.hpp>

#include "Engine.GFX/Memory/AllocationResult.hpp"
#include "Engine.GFX/Memory/VkAllocator.hpp"
#include "../Cache/GlobalCacheCtrl.hpp"
#include "../Cache/GlobalResourceCache.hpp"

using namespace ember::engine::gfx;
using namespace ember;

// Warning: Remove asap
bool tmpTestObject = false;

StaticGeometryLoader::StaticGeometryLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    GeometryLoader(cache_->cache()->device()),
    _cacheCtrl(cache_) {}

StaticGeometryLoader::~StaticGeometryLoader() = default;

StaticGeometryLoader::result_type StaticGeometryLoader::operator()(
    const ptr<assets::StaticGeometry> asset_,
    options_type options_
) {

    auto* ptr { _cacheCtrl->cache()->request(asset_) };

    if (!tmpTestObject) {
        //auto* ptr = new StaticGeometryResource {};
        loadWithAssimp(asset_, ptr);
        tmpTestObject = true;
    }

    /*
    if (!ptr->loaded()) {
        //auto* ptr = new StaticGeometryResource {};
        loadWithAssimp(asset_, ptr);
    }
     */

    return ptr;
}

constexpr auto test_fallback_filettype = "file"sv;
constexpr auto test_fallback_file = R"(R:\Development\C++\Vulkan API\Game\resources\assets\geometry\cylinder.obj)"sv;

void StaticGeometryLoader::loadWithAssimp(const ptr<assets::StaticGeometry> asset_,
    const ptr<StaticGeometryResource> dst_) {

    assert(dst_->_indexData.buffer->memory()->allocatedSize() > 0);
    assert(dst_->_vertexData.buffer->memory()->allocatedSize() > 0);

    Url src { ""sv, ""sv };

    if (asset_->sources().empty()) {
        src = Url { test_fallback_filettype, test_fallback_file };

    } else {
        src = asset_->sources().front();
    }

    /**
     *
     */
    Assimp::Importer importer {};
    aiPostProcessSteps ppFlags {};

    /**
     *
     */
    const aiScene* scene { importer.ReadFile(src.path().data(), ppFlags) };

    if (scene == nullptr) {
        DEBUG_SNMSG(false, "ASSIMP", importer.GetErrorString())
        return;// Failed to import
    }

    /**
     *
     */
    auto meshes = scene->mMeshes;
    auto meshCount = scene->mNumMeshes;
    for (auto i = 0; i < meshCount; ++i) {
        auto* mesh { meshes[i] };

        u32 fc { mesh->mNumFaces };

        Vector<u32> indices {};
        indices.reserve(fc * 3ui32);

        for (u32 j = 0; j < fc; ++j) {
            auto face { mesh->mFaces[j] };

            for (auto k = 0; k < face.mNumIndices; ++k) {
                indices.push_back(face.mIndices[k]);
            }
        }

        auto vc { mesh->mNumVertices };

        Vector<vertex> vertices {};
        vertices.reserve(vc);

        auto* colors { mesh->mColors[0] };
        auto* texCoords { mesh->mTextureCoords[0] };
        auto* normals { mesh->mNormals };

        for (u32 j = 0; j < vc; ++j) {
            auto entry { mesh->mVertices[j] };

            vertices.push_back(vertex {
                { entry.x, entry.y, entry.z },
                colors ?
                    vertex::color_type {
                        static_cast<u8>(colors[j].r * 255.0f),
                        static_cast<u8>(colors[j].g * 255.0f),
                        static_cast<u8>(colors[j].b * 255.0f)
                    } :
                    vertex::color_type {
                        255ui8,
                        255ui8,
                        255ui8
                    },
                texCoords ?
                    vertex::uvm_type { texCoords[j].x, texCoords[j].y, texCoords[j].z } :
                    math::vec3_zero,
                normals ?
                    vertex::normal_type { normals[j].x, normals[j].y, normals[j].z } :
                    math::vec3_up
            });
        }

        /**
         *
         */

        assert(dst_->_indexData.buffer->memory()->allocatedSize() >= sizeof(u32) * indices.size());
        assert(dst_->_vertexData.buffer->memory()->allocatedSize() >= sizeof(vertex) * vertices.size());

        {
            const auto indexSize { sizeof(u32) * indices.size() };
            auto* memory { dst_->_indexData.buffer->pages()[0]->memory()->allocated() };

            constexpr auto shift { 7ui64 };
            constexpr auto mask { 0b0111'1111ui64 };

            const auto aligned {
                ((indexSize >> shift) << shift) +
                ((indexSize & mask) ? + 1ui64 << shift : 0ui64)
            };

            const auto patchSize { _STD min(aligned, memory->size) };

            memory->map(patchSize);
            memory->write(indices.data(), indexSize);
            memory->flush(patchSize);
            memory->unmap();
        }

        {
            const auto vertexSize { sizeof(vertex) * vertices.size() };
            auto* memory { dst_->_vertexData.buffer->pages()[0]->memory()->allocated() };

            constexpr auto shift { 7ui64 };
            constexpr auto mask { 0b0111'1111ui64 };

            const auto aligned {
                ((vertexSize >> shift) << shift) +
                ((vertexSize & mask) ? + 1ui64 << shift : 0ui64)
            };

            const auto patchSize { _STD min(aligned, memory->size) };

            memory->map(patchSize);
            memory->write(vertices.data(), vertexSize);
            memory->flush(patchSize);
            memory->unmap();
        }
    }

    /**
     *
     */
    //delete scene;
}
