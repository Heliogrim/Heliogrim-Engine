#include "Cube.hpp"

#include "PolygonMesh.hpp"
#include "../Graphics.hpp"

using namespace hg::engine::gfx;
using namespace hg;

Cube::~Cube() {}

ptr<Mesh> Cube::convert() const {
    #if FALSE
    auto device = Graphics::get()->getCurrentDevice();

    /*
    _segment.u = 1;
    _segment.v = 1;
    */

    const u64 segCount = 6 * _segment.u * _segment.v;
    const u64 vertexCount = 8;
    const u64 indexCount = 6 * 6;

    vertex* vpt = new vertex[vertexCount];
    u32* ipt = new u32[indexCount];

    auto vp = &vpt[0];
    auto ip = &ipt[0];

    auto stepU = _scale.u / _segment.u;
    auto stepV = _scale.v / _segment.v;

    vp[0] = vertex {
        { 0.5F, 0.5F, 0.5F },
        math::vec3_t<u8> { 0u },
        { 0.F, 1.F, 0 },
        { 1.F, 1.F, 1.F }
    };

    vp[1] = vertex {
        { 0.5F, -0.5F, 0.5F },
        math::vec3_t<u8> { 0u },
        { 0.F, 0.F, 0 },
        { 1.F, -1.F, 1.F }
    };

    vp[2] = vertex {
        { -0.5F, -0.5F, 0.5F },
        math::vec3_t<u8> { 0u },
        { 1.F, 0.F, 0 },
        { -1.F, -1.F, 1.F }
    };

    vp[3] = vertex {
        { -0.5F, 0.5F, -0.5F },
        math::vec3_t<u8> { 0u },
        { 1.F, 1.F, 0 },
        { -1.F, 1.F, -1.F }
    };

    vp[4] = vertex {
        { 0.5F, 0.5F, -0.5F },
        math::vec3_t<u8> { 0u },
        { 1.F, 1.F, 0 },
        { 1.F, 1.F, -1.F }
    };

    vp[5] = vertex {
        { 0.5F, -0.5F, -0.5F },
        math::vec3_t<u8> { 0u },
        { 1.F, 0.F, 0 },
        { 1.F, -1.F, -1.F }
    };

    vp[6] = vertex {
        { -0.5F, -0.5F, -0.5F },
        math::vec3_t<u8> { 0u },
        { 0.F, 0.F, 0 },
        { -1.F, -1.F, -1.F }
    };

    vp[7] = vertex {
        { -0.5F, 0.5F, -0.5F },
        math::vec3_t<u8> { 0u },
        { 0.F, 1.F, 0 },
        { -1.F, 1.F, -1.F }
    };

    ip[0] = 0;
    ip[1] = 1;
    ip[2] = 2;

    ip[3] = 2;
    ip[4] = 3;
    ip[5] = 0;

    ip[6] = 4;
    ip[7] = 5;
    ip[8] = 6;

    ip[9] = 6;
    ip[10] = 7;
    ip[11] = 4;

    ip[12] = 0;
    ip[13] = 4;
    ip[14] = 7;

    ip[15] = 7;
    ip[16] = 3;
    ip[17] = 0;

    ip[18] = 1;
    ip[19] = 5;
    ip[20] = 6;

    ip[21] = 6;
    ip[22] = 2;
    ip[23] = 1;

    ip[24] = 0;
    ip[25] = 1;
    ip[26] = 5;

    ip[27] = 5;
    ip[28] = 4;
    ip[29] = 0;

    ip[30] = 7;
    ip[31] = 6;
    ip[32] = 2;

    ip[33] = 2;
    ip[34] = 3;
    ip[35] = 7;

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

    memcpy(vertexBuffer.mapped, vpt, vertexCount * sizeof(vertex));

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

    memcpy(indexBuffer.mapped, ipt, indexCount * sizeof(u32));

    indexBuffer.unmap();
    delete[] ipt;

    return mesh;
    #endif
    return nullptr;
}

ref<math::uivec2> Cube::segments() noexcept {
    return _segment;
}

ref<math::fvec2> Cube::scale() noexcept {
    return _scale;
}
