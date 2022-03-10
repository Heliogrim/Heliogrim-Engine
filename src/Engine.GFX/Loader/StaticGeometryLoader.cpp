#include "StaticGeometryLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Engine.Common/Math/Coordinates.hpp>

#include "Engine.GFX/Memory/VkAllocator.hpp"

using namespace ember::engine::gfx;
using namespace ember;

// Warning: Remove asap
ptr<StaticGeometryResource> tmpTestObject = nullptr;

StaticGeometryLoader::StaticGeometryLoader(cref<sptr<Device>> device_) :
    GeometryLoader(device_) {}

StaticGeometryLoader::~StaticGeometryLoader() {

    if (tmpTestObject) {
        tmpTestObject->_indexData.buffer.destroy();
        tmpTestObject->_vertexData.buffer.destroy();
        delete tmpTestObject;
    }

}

StaticGeometryLoader::result_type StaticGeometryLoader::operator()(
    const ptr<assets::StaticGeometry> asset_,
    options_type options_
) {

    if (tmpTestObject) {
        return tmpTestObject;
    }

    auto* ptr = new StaticGeometryResource {};
    loadWithAssimp(asset_, ptr);

    //
    tmpTestObject = ptr;

    return ptr;
}

void StaticGeometryLoader::loadWithAssimp(const ptr<assets::StaticGeometry> asset_,
    const ptr<StaticGeometryResource> dst_) {

    // TODO: Replace
    const Url src { "file"sv, R"(R:\Development\C++\Vulkan API\Game\resources\assets\geometry\cube.obj)"sv };

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

        {
            vk::BufferCreateInfo bci {
                vk::BufferCreateFlags {},
                sizeof(u32) * indices.size(),
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                vk::SharingMode::eExclusive,
                0,
                nullptr
            };

            IndexBuffer ib {};
            ib.buffer = _device->vkDevice().createBuffer(bci);
            ib.usageFlags = bci.usage;
            ib.device = _device->vkDevice();
            ib.size = bci.size;

            auto* alloc {
                VkAllocator::makeForBuffer(_device, ib.buffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible)
            };

            ib.memory = alloc->allocate(bci.size);
            delete alloc;

            //
            ib.map();
            ib.memory->write(indices.data(), indices.size() * sizeof(u32));
            ib.unmap();

            //
            ib.bind();
            dst_->_indexData.buffer = ib;
        }

        {
            vk::BufferCreateInfo bci {
                vk::BufferCreateFlags {},
                sizeof(vertex) * vertices.size(),
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                vk::SharingMode::eExclusive,
                0,
                nullptr
            };

            VertexBuffer vb {};
            vb.buffer = _device->vkDevice().createBuffer(bci);
            vb.usageFlags = bci.usage;
            vb.device = _device->vkDevice();
            vb.size = bci.size;

            auto* alloc {
                VkAllocator::makeForBuffer(_device, vb.buffer,
                    vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible)
            };

            vb.memory = alloc->allocate(bci.size);
            delete alloc;

            //
            vb.map();
            vb.memory->write(vertices.data(), vertices.size() * sizeof(vertex));
            vb.unmap();

            //
            vb.bind();
            dst_->_vertexData.buffer = vb;
        }
    }

    /**
     *
     */
    //delete scene;
}
