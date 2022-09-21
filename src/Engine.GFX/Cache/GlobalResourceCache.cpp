#include "GlobalResourceCache.hpp"

#include "Engine.GFX/Geometry/Vertex.hpp"
#include "Engine.GFX/Memory/AllocationResult.hpp"
#include "../Texture/TextureFactory.hpp"
#include "../Material/MaterialMetaDto.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember::engine::gfx;
using namespace ember;

ptr<GlobalResourceCache> GlobalResourceCache::_instance = nullptr;

GlobalResourceCache::GlobalResourceCache(cref<sptr<Device>> device_) :
    _device(device_),
    _atlasMaxLayers(128ui32),
    _atlasMaxLayerExtent({ 8192ui32 }),
    _atlasMinLayerExtent({ 16ui32 }),
    _atlasMaxSpatialExtent({ 0ui32 }),
    _atlasMinSpatialExtent({ 0ui32 }) {}

GlobalResourceCache::~GlobalResourceCache() = default;

ptr<GlobalResourceCache> GlobalResourceCache::make(cref<sptr<Device>> device_) {
    assert(!GlobalResourceCache::_instance);

    _instance = new GlobalResourceCache(device_);
    return _instance;
}

void GlobalResourceCache::destroy() {

    /**
     * Release unique pointers to virtual buffers for vertices
     */
    for (auto& entry : _instance->_vertexBuffers) {
        entry.reset();
    }

    /**
     * Release unique pointers to virtual buffers for indices
     */
    for (auto& entry : _instance->_indexBuffers) {
        entry.reset();
    }

    /**
     * Clear mappings and lists
     */
    _instance->_mapped.clear();
    _instance->_indexBuffers.clear();
    _instance->_vertexBuffers.clear();

    /**
     *
     */
    delete _instance;
    _instance = nullptr;
}

cref<sptr<Device>> GlobalResourceCache::device() const noexcept {
    return _device;
}

bool GlobalResourceCache::hook(cref<asset_guid> guid_, ptr<StaticGeometryResource> resource_) {

    /**
     * Hook must be unique
     */
    if (_mapped.find(guid_) != _mapped.end()) {
        return false;
    }

    auto managed { resource_->acquire(res::ResourceUsageFlag::eAll) };
    managed.release();

    /**
     * Store hooked resource
     */
    _mapped.insert_or_assign(guid_, resource_);
    return true;
}

bool GlobalResourceCache::unhook(cref<asset_guid> guid_, ptr<StaticGeometryResource> resource_) {
    return false;
}

ptr<StaticGeometryResource> GlobalResourceCache::request(const ptr<const assets::StaticGeometry> asset_) {

    /**
     * Check whether resource does already exists
     */
    auto iter { _mapped.find(asset_->get_guid()) };
    if (iter != _mapped.end()) {
        return static_cast<ptr<StaticGeometryResource>>(iter->second);
    }

    /**
     *
     */
    auto* res { new StaticGeometryResource() };

    auto indexCount { asset_->getIndexCount() };
    auto vertexCount { asset_->getVertexCount() };

    auto* vib { requestIndexBuffer({ indexCount * sizeof(u32) }) };
    auto* vvb { requestVertexBuffer({ vertexCount * sizeof(vertex) }) };

    // TODO: Get virtual buffer view for indexes
    // TODO: Get virtual buffer view for vertices

    //res->_indexData = indexView;
    //res->_vertexData = vertexView;
    res->_indexData.buffer = vib;
    res->_vertexData.buffer = vvb;

    /**
     * Store requested resource
     */
    _mapped.insert_or_assign(asset_->get_guid(), res);
    return res;
}

ptr<TextureResource> GlobalResourceCache::request(const ptr<const assets::Texture> asset_) {

    /**
     * Check whether resource does already exists
     */
    auto iter { _mapped.find(asset_->get_guid()) };
    if (iter != _mapped.end()) {
        return static_cast<ptr<TextureResource>>(iter->second);
    }

    const auto& extent { asset_->getExtent() };
    u32 layers { 1ui32 };
    const auto format { asset_->getTextureFormat() };
    const auto type { asset_->getTextureType() };

    // Warning: Temporary Solution
    if (type == TextureType::eCube && extent.z == 1ui32) {
        layers = 6ui32;
    } else if (extent.z != 1ui32 || type != TextureType::e2d) {
        // Warning: Currently only support single layer 2d images
        __debugbreak();
        return nullptr;
    }

    assert(extent.x <= _atlasMaxLayerExtent.x);
    assert(extent.y <= _atlasMaxLayerExtent.y);

    /**
     * Try find a suitable texture atlas
     */
    ptr<VirtualTexture> atlas { nullptr };

    for (const auto& candidate : _textureAtlas) {

        const auto hasMip { candidate->mipLevels() > 1ui32 };
        const auto isFormat { candidate->format() == format };
        const auto isExtent {
            hasMip ?
                (extent.x <= candidate->width() && extent.y <= candidate->height()) :
                (extent.x == candidate->width() && extent.y == candidate->height())
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

        atlas = TextureFactory::get()->buildVirtual({
            layers/* TODO: Layers */,
            extent,
            format,
            math::uivec2 { 0ui32, asset_->getMipLevelCount() - 1ui32 },
            type == TextureType::eCube ? TextureType::eCube : TextureType::e2dArray,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        });

        auto uatlas { _STD unique_ptr<VirtualTexture>(atlas) };
        _textureAtlas.push_back(_STD move(uatlas));
    }

    /**
     *
     */
    auto view { atlas->makeView({ 0ui32, layers - 1ui32 }, { 0ui32, asset_->getMipLevelCount() - 1ui32 }) };

    auto* res { make_ptr<TextureResource>() };
    res->setOrigin(asset_);
    res->_payload.view = _STD move(view);

    /**
     * Store requested resource
     */
    _mapped.insert_or_assign(asset_->get_guid(), res);
    return res;
}

ptr<MaterialResource> GlobalResourceCache::request(const ptr<const assets::GfxMaterial> asset_) {
    /**
     * Check whether resource does already exists
     */
    auto iter { _mapped.find(asset_->get_guid()) };
    if (iter != _mapped.end()) {
        return static_cast<ptr<MaterialResource>>(iter->second);
    }

    /**
     *
     */
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

    auto view { buffer->makeView(/*(forward++) * dataSize*/0ui32, dataSize) };

    auto* res { new MaterialResource() };
    res->setOrigin(asset_);
    res->_payload.view = _STD move(view);

    /**
     * Store requested resource
     */
    _mapped.insert_or_assign(asset_->get_guid(), res);
    return res;
}

void GlobalResourceCache::drop(cref<asset_guid> guid_, mref<ptr<StaticGeometryResource>> resource_) {

    /**
     * Check whether resource exists
     */
    auto iter { _mapped.find(guid_) };
    if (iter == _mapped.end() || iter->second != resource_) {
        return;
    }

    /**
     *
     */
    // TODO: Erase resource_->_indexData as virtual buffer view from indexes
    // TODO: Erase resource_->_vertexData as virtual buffer view from vertices

    /**
     * Cleanup
     */
    _mapped.erase(iter);
    delete resource_;
}

non_owning_rptr<VirtualBuffer> GlobalResourceCache::requestVertexBuffer(mref<Vector<u64>> pageSizes_) {

    u64 wholeSize { 0ui64 };
    for (auto pageSize : pageSizes_) {
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

    return ptr;
}

non_owning_rptr<VirtualBuffer> GlobalResourceCache::requestIndexBuffer(mref<Vector<u64>> pageSizes_) {

    u64 wholeSize { 0ui64 };
    for (auto pageSize : pageSizes_) {
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

    return ptr;
}
