#include "VirtualTexturePage.hpp"

#include "../Memory/AllocatedMemory.hpp"
#include "../Memory/AllocationResult.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualTexturePage::VirtualTexturePage(
    non_owning_rptr<VirtualMemoryPage> memory_,
    u32 layer_,
    math::uivec3 offset_,
    math::uivec3 extent_,
    u32 mipLevel_,
    const VirtualTexturePageFlags flags_
) :
    _flags(flags_),
    _memory(memory_),
    _layer(layer_),
    _offset(offset_),
    _extent(extent_),
    _mipLevel(mipLevel_) {}

VirtualTexturePage::~VirtualTexturePage() noexcept {
    delete _memory;
}

const VirtualTexturePageFlags VirtualTexturePage::flags() const noexcept {
    return _flags;
}

const non_owning_rptr<VirtualMemoryPage> VirtualTexturePage::memory() const noexcept {
    return _memory;
}

bool VirtualTexturePage::residential() const noexcept {
    return false;
}

bool VirtualTexturePage::load() {

    const auto result { _memory->load() };
    if (result != memory::AllocationResult::eSuccess) {
        return false;
    }

    return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool VirtualTexturePage::unload() {
    _memory->unload();
    return _memory->state() == VirtualMemoryPageState::eUnloaded;
}

u32 VirtualTexturePage::layer() const noexcept {
    return _layer;
}

cref<math::uivec3> VirtualTexturePage::offset() const noexcept {
    return _offset;
}

cref<math::uivec3> VirtualTexturePage::extent() const noexcept {
    return _extent;
}

u32 VirtualTexturePage::mipLevel() const noexcept {
    return _mipLevel;
}

vk::SparseImageMemoryBind VirtualTexturePage::vkSparseImageMemoryBind() const noexcept {

    const bool isMemoryBacking { _memory->state() == VirtualMemoryPageState::eLoaded };

    return vk::SparseImageMemoryBind {
        vk::ImageSubresource {
            vk::ImageAspectFlagBits::eColor,
            _mipLevel,
            _layer
        },
        vk::Offset3D { INT32_T(_offset.x), INT32_T(_offset.y), INT32_T(_offset.z) },
        vk::Extent3D { _extent.x, _extent.y, _extent.z },
        isMemoryBacking ? _memory->allocated()->vkMemory : nullptr,
        isMemoryBacking ? _memory->allocated()->offset : 0ui64,
        vk::SparseMemoryBindFlags {}
    };
}

vk::SparseMemoryBind VirtualTexturePage::vkSparseMemoryBind() const noexcept {

    const bool isMemoryBacking { _memory->state() == VirtualMemoryPageState::eLoaded };

    return vk::SparseMemoryBind {
        _memory->offset(),
        _memory->size(),
        isMemoryBacking ? _memory->allocated()->vkMemory : nullptr,
        isMemoryBacking ? _memory->allocated()->offset : 0ui64,
        vk::SparseMemoryBindFlags {}
    };

}
