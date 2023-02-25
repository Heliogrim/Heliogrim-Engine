#include "GlobalResourcePool.hpp"

#include "../Buffer/VirtualBufferView.hpp"
#include "../Texture/VirtualTextureView.hpp"
#include "../Material/MaterialMetaDto.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx::pool;
using namespace ember;

GlobalResourcePool::GlobalResourcePool(cref<sptr<Device>> device_) :
    _device(device_),
    _atlasMaxLayers(128ui32),
    _atlasMaxLayerExtent({ 8192ui32 }),
    _atlasMinLayerExtent({ 16ui32 }),
    _atlasMaxSpatialExtent({ 0ui32 }),
    _atlasMinSpatialExtent({ 0ui32 }) {}

GlobalResourcePool::~GlobalResourcePool() = default;

void GlobalResourcePool::setup() {}

void GlobalResourcePool::destroy() {

    /**
     * Release unique pointers to virtual buffers for vertices
     */
    for (auto& entry : _vertexBuffers) {
        entry.reset();
    }

    /**
     * Release unique pointers to virtual buffers for indices
     */
    for (auto& entry : _indexBuffers) {
        entry.reset();
    }

    /**
     * Clear lists
     */
    _indexBuffers.clear();
    _vertexBuffers.clear();
}

cref<sptr<engine::gfx::Device>> GlobalResourcePool::device() const noexcept {
    return _device;
}

uptr<engine::gfx::VirtualBufferView> GlobalResourcePool::allocateIndexBuffer(
    mref<IndexBufferAllocation> allocation_
) {

    u64 wholeSize { 0ui64 };
    for (auto pageSize : allocation_.pageSizes) {
        wholeSize += pageSize;
    }

    assert(wholeSize > 0ui64);

    /**
     * Create sparse vulkan buffer
     */
    vk::BufferCreateInfo ci {
        vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
        wholeSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        0ui32,
        nullptr,
        nullptr
    };

    auto buffer { _device->vkDevice().createBuffer(ci) };

    /**
     * Store page meta data and allocate
     */

    // Warning: The following memory management only works as long as the requested data size is less or equal to the memory requirements alignment
    const auto req { _device->vkDevice().getBufferMemoryRequirements(buffer) };
    const memory::MemoryLayout layout {
        req.alignment,
        MemoryProperty::eDeviceLocal | MemoryProperty::eHostVisible,
        req.memoryTypeBits
    };

    /**
     * Get suitable virtual memory instance
     */
    auto memory {
        make_uptr<VirtualMemory>(
            _device->allocator(),
            layout,
            req.size
        )
    };

    /**
     * Gather virtual buffer instance
     */
    auto vb {
        make_uptr<VirtualBuffer>(
            _STD move(memory),
            buffer,
            ci.usage
        )
    };

    /*
    u64 offset { 0ui64 };
    s64 leftPageSize { static_cast<s64>(req.alignment) };
    for (const auto pageSize : pageSizes_) {

        if ((leftPageSize -= pageSize) > 0) {
            continue;
        }

        // TBD:
        __debugbreak();
    }
     */

    const auto required {
        (wholeSize / req.alignment) +
        ((wholeSize % req.alignment) ? 1ui64 : 0ui64)
    };

    for (u64 page { 0ui64 }; page < required; ++page) {
        auto vp { vb->addPage(req.alignment, req.alignment * page) };

        #ifdef _DEBUG
        assert(vp->load());
        #else
        vp->load();
        #endif
    }

    /**
     * Bind buffer and allocated memory
     */
    vb->updateBindingData();

    #pragma warning(push)
    #pragma warning(disable : 4996)
    vb->enqueueBindingSync(_device->graphicsQueue());
    #pragma warning(pop)

    /**
     * Store and return virtual buffer
     */
    auto* ptr { vb.get() };
    _indexBuffers.push_back(_STD move(vb));

    return ptr->makeView(/*offset*/0ui64, /*size*/ptr->memory()->size());
}

uptr<engine::gfx::VirtualBufferView> GlobalResourcePool::allocateVertexBuffer(
    mref<VertexBufferAllocation> allocation_
) {

    u64 wholeSize { 0ui64 };
    for (auto pageSize : allocation_.pageSizes) {
        wholeSize += pageSize;
    }

    assert(wholeSize > 0ui64);

    /**
     * Create sparse vulkan buffer
     */
    vk::BufferCreateInfo ci {
        vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
        wholeSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        0ui32,
        nullptr,
        nullptr
    };

    auto buffer { _device->vkDevice().createBuffer(ci) };

    /**
     * Store page meta data and allocate
     */

    // Warning: The following memory management only works as long as the requested data size is less or equal to the memory requirements alignment
    const auto req { _device->vkDevice().getBufferMemoryRequirements(buffer) };
    const memory::MemoryLayout layout {
        req.alignment,
        MemoryProperty::eDeviceLocal | MemoryProperty::eHostVisible,
        req.memoryTypeBits
    };

    /**
     * Get suitable virtual memory instance
     */
    auto memory {
        make_uptr<VirtualMemory>(
            _device->allocator(),
            layout,
            req.size
        )
    };

    /**
     * Gather virtual buffer instance
     */
    auto vb {
        make_uptr<VirtualBuffer>(
            _STD move(memory),
            buffer,
            ci.usage
        )
    };

    /*
    u64 offset { 0ui64 };
    s64 leftPageSize { static_cast<s64>(req.alignment) };
    for (const auto pageSize : pageSizes_) {

        if ((leftPageSize -= pageSize) > 0) {
            continue;
        }

        // TBD:
        __debugbreak();
    }
     */

    const auto required {
        (wholeSize / req.alignment) +
        ((wholeSize % req.alignment) ? 1ui64 : 0ui64)
    };

    for (u64 page { 0ui64 }; page < required; ++page) {
        auto vp { vb->addPage(req.alignment, req.alignment * page) };

        #ifdef _DEBUG
        assert(vp->load());
        #else
        vp->load();
        #endif
    }

    /**
     * Bind buffer and allocated memory
     */
    vb->updateBindingData();

    #pragma warning(push)
    #pragma warning(disable : 4996)
    vb->enqueueBindingSync(_device->graphicsQueue());
    #pragma warning(pop)

    /**
     * Store and return virtual buffer
     */
    auto* ptr { vb.get() };
    _vertexBuffers.push_back(_STD move(vb));

    return ptr->makeView(/*offset*/0ui64, /*size*/ptr->memory()->size());
}

uptr<engine::gfx::VirtualBufferView> GlobalResourcePool::allocateVirtualBuffer(
    mref<MaterialBufferAllocation> allocation_
) {

    ptr<VirtualBuffer> buffer { nullptr };
    if (buffer == nullptr) {

        // Warning: Experimental
        constexpr const auto dataSize { sizeof(experimental::MaterialMetaDto) };

        /**
         * Create sparse vulkan buffer
         */
        vk::BufferCreateInfo ci {
            vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
            dataSize,
            vk::BufferUsageFlagBits::eUniformBuffer /* vk::BufferUsageFlagBits::eStorageBuffer */ |
            vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive,
            0ui32,
            nullptr,
            nullptr
        };

        auto vkBuffer { _device->vkDevice().createBuffer(ci) };

        /**
         * Store page meta data and allocate
         */

        // Warning: The following memory management only works as long as the requested data size is less or equal to the memory requirements alignment
        const auto req { _device->vkDevice().getBufferMemoryRequirements(vkBuffer) };
        const memory::MemoryLayout layout {
            req.alignment,
            MemoryProperty::eDeviceLocal,
            req.memoryTypeBits
        };

        /**
         * Get suitable virtual memory instance
         */
        auto memory {
            make_uptr<VirtualMemory>(
                _device->allocator(),
                layout,
                req.size
            )
        };

        /**
         * Gather virtual buffer instance
         */
        buffer = make_ptr<VirtualBuffer>(
            _STD move(memory),
            vkBuffer,
            ci.usage
        );

        const auto required {
            (dataSize / req.alignment) +
            ((dataSize % req.alignment) ? 1ui64 : 0ui64)
        };

        for (u64 page { 0ui64 }; page < required; ++page) {
            auto vp { buffer->addPage(req.alignment, req.alignment * page) };

            #ifdef _DEBUG
            assert(vp->load());
            #else
        vp->load();
            #endif
        }

        /**
         * Bind buffer and allocated memory
         */
        buffer->updateBindingData();

        #pragma warning(push)
        #pragma warning(disable : 4996)
        buffer->enqueueBindingSync(_device->graphicsQueue());
        #pragma warning(pop)

        /**
         * Store and return virtual buffer
         */
        auto ubuffer { _STD unique_ptr<VirtualBuffer>(buffer) };
        _materialBuffer.push_back(_STD move(ubuffer));
        _materialForward.push_back({});
    }

    /**
     *
     */
    const auto dataSize { sizeof(experimental::MaterialMetaDto) };
    auto& forward { _materialForward.front() };

    return buffer->makeView(/*(forward++) * dataSize*/0ui32, dataSize);
}

uptr<engine::gfx::VirtualTextureView> GlobalResourcePool::allocateVirtualTexture(
    mref<VirtualTextureAllocation> allocation_
) {
    u32 layers { 1ui32 };

    // Warning: Temporary Solution
    if (allocation_.type == TextureType::eCube && allocation_.extent.z == 1ui32) {
        layers = 6ui32;
    } else if (allocation_.extent.z != 1ui32 || allocation_.type != TextureType::e2d) {
        // Warning: Currently only support single layer 2d images
        __debugbreak();
        return {};
    }

    assert(allocation_.extent.x <= _atlasMaxLayerExtent.x);
    assert(allocation_.extent.y <= _atlasMaxLayerExtent.y);

    /**
     * Try find a suitable texture atlas
     */
    ptr<VirtualTexture> atlas { nullptr };

    for (const auto& candidate : _textureAtlas) {

        const auto hasMip { candidate->mipLevels() > 1ui32 };
        const auto isFormat { candidate->format() == allocation_.format };
        const auto isExtent {
            hasMip ?
                (allocation_.extent.x <= candidate->width() && allocation_.extent.y <= candidate->height()) :
                (allocation_.extent.x == candidate->width() && allocation_.extent.y == candidate->height())
        };

        if (isFormat && isExtent) {
            atlas = candidate.get();
        }
    }

    /**
     *
     */
    // TODO:
    atlas = nullptr;
    if (atlas == nullptr) {

        /**/
        TextureType payloadType { allocation_.type == TextureType::eCube ? TextureType::eCube : TextureType::e2dArray };
        if (allocation_.format == TextureFormat::eR8G8B8A8Srgb) {
            payloadType = allocation_.type;
        }
        /**/

        atlas = TextureFactory::get()->buildVirtual(
            {
                layers/* TODO: Layers */,
                allocation_.extent,
                allocation_.format,
                allocation_.mipLevels,
                payloadType,
                vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        );

        auto uatlas { _STD unique_ptr<VirtualTexture>(atlas) };
        _textureAtlas.push_back(_STD move(uatlas));
    }

    /**/

    auto view = atlas->makeView({ 0ui32, layers - 1ui32 }, allocation_.mipLevels);

    // Attention: This is just a workaround to prevent unfitting view bindings. We need to checkout a better solution...
    if (allocation_.type == TextureType::e2d && atlas->type() == TextureType::e2dArray) {
        TextureFactory::get()->buildView(*view, { .type = TextureType::e2d });
    }

    return view;
}
