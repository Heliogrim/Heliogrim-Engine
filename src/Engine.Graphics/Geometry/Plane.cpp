#include "Plane.hpp"

#include "PolygonMesh.hpp"
#include "Vertex.hpp"
#include "../Graphics.hpp"

using namespace hg::engine::gfx;
using namespace hg;

Plane::Plane() :
	_grid({ 1 }),
	_scale({ 1.F }) {}

Plane::~Plane() = default;

ptr<Mesh> Plane::convert() const {
	#if FALSE
    const auto device = Graphics::get()->getCurrentDevice().get();

    const u64 gridCount = _grid.u * _grid.v;
    const u64 vertexCount = gridCount + _grid.u + _grid.v + 1;
    const u64 indexCount = gridCount * 3 * 2;

    auto* vpt = new vertex[vertexCount];
    auto* ipt = new u32[indexCount];

    vertex* vp = &vpt[0];
    u32* ip = &ipt[0];

    const auto stepU = _scale.u / _grid.u;
    const auto stepV = _scale.v / _grid.v;

    for (u32 v = 0; v <= _grid.v; ++v) {
        for (u32 u = 0; u <= _grid.u; ++u) {
            *vp = vertex {
                vertex::position_type { static_cast<float>(u) * stepU, 0.F, static_cast<float>(v) * stepV },
                vertex::color_type { 0u },
                vertex::uvm_type {
                    static_cast<float>(u) / static_cast<float>(_grid.u),
                    static_cast<float>(v) / static_cast<float>(_grid.v),
                    0.F
                },
                vertex::normal_type { 0.F, 1.F, 0.0F }
            };

            ++vp;
        }
    }

    for (u32 v = 0; v < _grid.v; ++v) {
        for (u32 u = 0; u < _grid.u; ++u) {
            /**
             * Count of Vertex per Row
             */
            const auto rc = (_grid.u + 1);

            /**
             * Upper Right -> Lower Right -> Lower Left
             */
            *(ip + 0) = (v * rc) + u + 1;
            *(ip + 1) = ((v + 1) * rc) + u + 1;
            *(ip + 2) = ((v + 1) * rc) + u;

            ip += 3;

            /**
             * Lower Left -> Upper Left -> Upper Right
             */
            *(ip + 0) = ((v + 1) * rc) + u;
            *(ip + 1) = (v * rc) + u;
            *(ip + 2) = (v * rc) + u + 1;

            ip += 3;
        }
    }

    auto* mesh = new PolygonMesh {};
    auto& vertexBuffer = mesh->vertices();
    auto& indexBuffer = mesh->indices();

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        vertexCount * sizeof(vertex),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    vertexBuffer.buffer = device->vkDevice().createBuffer(bci);

    vk::MemoryRequirements mr = device->vkDevice().getBufferMemoryRequirements(vertexBuffer.buffer);
    vk::MemoryAllocateInfo mai {
        mr.size,
        get_memory_type(device->vkPhysicalDevice(), mr.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible)
    };

    vertexBuffer.memory = device->vkDevice().allocateMemory(mai);

    assert(vertexBuffer.buffer);
    assert(vertexBuffer.memory);

    vertexBuffer.device = device->vkDevice();
    vertexBuffer.size = mr.size;
    vertexBuffer.align = mr.alignment;

    vertexBuffer.bind();
    vertexBuffer.map();

    std::memcpy(vertexBuffer.mapped, vpt, vertexCount * sizeof(vertex));

    vertexBuffer.unmap();
    delete[] vpt;

    bci = vk::BufferCreateInfo {
        vk::BufferCreateFlags(),
        indexCount * sizeof(u32),
        vk::BufferUsageFlagBits::eIndexBuffer,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    indexBuffer.buffer = device->vkDevice().createBuffer(bci);

    mr = device->vkDevice().getBufferMemoryRequirements(indexBuffer.buffer);
    mai = vk::MemoryAllocateInfo {
        mr.size,
        get_memory_type(device->vkPhysicalDevice(), mr.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible)
    };

    indexBuffer.memory = device->vkDevice().allocateMemory(mai);

    assert(indexBuffer.buffer);
    assert(indexBuffer.memory);

    indexBuffer.device = device->vkDevice();
    indexBuffer.size = mr.size;
    indexBuffer.align = mr.alignment;

    indexBuffer.bind();
    indexBuffer.map();

    std::memcpy(indexBuffer.mapped, ipt, indexCount * sizeof(u32));

    indexBuffer.unmap();
    delete[] ipt;

    return mesh;
	#endif
	return nullptr;
}

ref<math::uivec2> Plane::grid() noexcept {
	return _grid;
}

ref<math::fvec2> Plane::scale() noexcept {
	return _scale;
}
