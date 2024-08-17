#include "SparseTexturePage.hpp"

#include <utility>

#include "../Memory/AllocatedMemory.hpp"
#include "../Memory/AllocationResult.hpp"
#include "Engine.GFX/Memory/VirtualMemory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseTexturePage::SparseTexturePage(
	nmpt<VirtualMemoryPage> memory_,
	u32 layer_,
	math::uivec3 offset_,
	math::uivec3 extent_,
	u32 mipLevel_,
	const SparseTexturePageFlags flags_
) :
	_flags(flags_),
	_memory(memory_),
	_layer(layer_),
	_offset(offset_),
	_extent(extent_),
	_mipLevel(mipLevel_) {}

SparseTexturePage::~SparseTexturePage() noexcept {
	// delete _memory;
	assert(_memory == nullptr);
}

const SparseTexturePageFlags SparseTexturePage::flags() const noexcept {
	return _flags;
}

nmpt<VirtualMemoryPage> SparseTexturePage::release() noexcept {
	return std::exchange(_memory, nullptr);
}

nmpt<VirtualMemoryPage> SparseTexturePage::memory() const noexcept {
	return _memory;
}

bool SparseTexturePage::residential() const noexcept {
	return false;
}

bool SparseTexturePage::load() {

	const auto result { _memory->load() };
	if (result != memory::AllocationResult::eSuccess) {
		return false;
	}

	return _memory->state() == VirtualMemoryPageState::eLoaded;
}

bool SparseTexturePage::unload() {
	_memory->unload();
	return _memory->state() == VirtualMemoryPageState::eUnloaded;
}

u32 SparseTexturePage::layer() const noexcept {
	return _layer;
}

cref<math::uivec3> SparseTexturePage::offset() const noexcept {
	return _offset;
}

cref<math::uivec3> SparseTexturePage::extent() const noexcept {
	return _extent;
}

u32 SparseTexturePage::mipLevel() const noexcept {
	return _mipLevel;
}

vk::SparseImageMemoryBind SparseTexturePage::vkSparseImageMemoryBind() const noexcept {

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
		isMemoryBacking ? _memory->allocated()->offset : 0uLL,
		vk::SparseMemoryBindFlags {}
	};
}

vk::SparseMemoryBind SparseTexturePage::vkSparseMemoryBind() const noexcept {

	const bool isMemoryBacking { _memory->state() == VirtualMemoryPageState::eLoaded };

	return vk::SparseMemoryBind {
		_memory->offset(),
		_memory->size(),
		isMemoryBacking ? _memory->allocated()->vkMemory : nullptr,
		isMemoryBacking ? _memory->allocated()->offset : 0uLL,
		vk::SparseMemoryBindFlags {}
	};

}
