#include "SparseTexture.hpp"

#include <ranges>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>

#include "SparseTexturePage.hpp"
#include "SparseTextureView.hpp"

// TODO: Remove
#include "Engine.Core/Engine.hpp"
#include "Engine.GFX/Graphics.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseTexture::SparseTexture(
	mref<uptr<VirtualMemory>> memory_,
	const u32 layers_,
	const math::uivec3 extent_,
	const TextureFormat format_,
	const math::uivec2 mipLevels_,
	const TextureType type_,
	vk::Image vkImage_
) :
	_memory(std::move(memory_)),
	_layers(layers_),
	_extent(extent_),
	_format(format_),
	_mipLevels(mipLevels_),
	_type(type_),
	_mipTailFirstLod(0uL),
	_mipTailSize(0uL),
	_mipTailOffset(0uL),
	_mipTailStride(0uL),
	_vkImage(vkImage_) {}

SparseTexture::SparseTexture(
	mref<uptr<VirtualMemory>> memory_,
	u32 layers_,
	math::uivec3 extent_,
	TextureFormat format_,
	math::uivec2 mipLevels_,
	TextureType type_,
	vk::Image vkImage_,
	math::uivec3 granularity_,
	u32 mipTailFirstLod_,
	u32 mipTailSize_,
	u32 mipTailOffset_,
	u32 mipTailStride_
) :
	_memory(std::move(memory_)),
	_layers(layers_),
	_extent(extent_),
	_format(format_),
	_mipLevels(mipLevels_),
	_type(type_),
	_granularity(granularity_),
	_mipTailFirstLod(mipTailFirstLod_),
	_mipTailSize(mipTailSize_),
	_mipTailOffset(mipTailOffset_),
	_mipTailStride(mipTailStride_),
	_vkImage(vkImage_) {}

SparseTexture::~SparseTexture() {
	tidy();
}

void SparseTexture::tidy() {

	/**
	 * Destroy Image before backing memory or pages
	 */
	if (_vkImage) {
		Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyImage(_vkImage);
	}

	/**
	 * Delete every virtual texture page and implicitly pages virtual memory before backing memory
	 */
	for (auto& page : _pages) {
		/* Memory Pages are getting freed by follow-up memory destructor */
		page->release();
		page.reset();
	}

	_pages.clear();

	for (auto&& page : _opaquePages) {
		/* Memory Pages are getting freed by follow-up memory destructor */
		page->release();
		page.reset();
	}

	_opaquePages.clear();

	/**
	 * Delete virtual memory and it's descendents
	 */
	_memory.reset();
}

[[nodiscard]] u64 formatDataSize(TextureFormat format_) {
	switch (format_) {
		case TextureFormat::eR16Sfloat: {
			return {
				16uLL/* Bits per Channel */
				* 1uLL/* Channels */
				/ 8uLL/* Bits to Bytes*/
			};
		}
		case TextureFormat::eR8G8B8A8Unorm: {
			return {
				8uLL/* Bits per Channel */
				* 4uLL/* Channels */
				/ 8uLL/* Bits to Bytes*/
			};
		}
		case TextureFormat::eR32G32B32A32Sfloat: {
			return {
				32uLL/* Bits per Channel */
				* 4uLL/* Channels */
				/ 8uLL/* Bits to Bytes*/
			};
		}
		default: {
			::hg::breakpoint();
			return 0uLL;
		}
	}
}

nmpt<SparseTexturePage> SparseTexture::makePage(
	u32 layer_,
	u32 mipLevel_,
	math::uivec3 tileOffset_,
	math::uivec3 tileExtent_
) {

	assert(
		_format == TextureFormat::eR8Unorm ||
		_format == TextureFormat::eR8G8B8Unorm ||
		_format == TextureFormat::eR8G8B8A8Unorm ||
		_format == TextureFormat::eR8G8B8A8Srgb ||
		_format == TextureFormat::eR16G16B16A16Sfloat ||
		_format == TextureFormat::eR32G32B32A32Sfloat ||
		_format == TextureFormat::eR16Sfloat
	);

	#ifdef _DEBUG
	assert(_layers >= layer_);
	assert(_mipLevels.min <= mipLevel_ && mipLevel_ <= _mipLevels.max);
	#endif

	/**
	 *
	 */
	nmpt<VirtualMemoryPage> memory {};

	const auto formatSize { formatDataSize(_format) };
	assert(formatSize > 0);

	const auto memorySize {
		formatSize * (tileExtent_.x * tileExtent_.y * tileExtent_.z)
	};
	const auto memoryOffset { ~0uLL };

	memory = _memory->definePage(memoryOffset, memorySize);
	assert(!!memory);

	/**
	 *
	 */
	auto page = uptr<SparseTexturePage>(
		new SparseTexturePage(
			memory,
			layer_,
			tileOffset_,
			tileExtent_,
			mipLevel_
		)
	);

	auto* const result = page.get();
	_pages.emplace_back(std::move(page));
	return result;
}

nmpt<SparseTexturePage> SparseTexture::makeOpaquePage(u32 layer_) {

	#ifdef _DEBUG
	assert(_layers >= layer_);
	#endif

	auto extent { _extent };
	for (u32 mip { 0 }; mip < _mipTailFirstLod; ++mip) {
		extent.x >>= 1uL;
		extent.y >>= 1uL;
		//extent.z >>= 1uL;
	}

	/**
	 *
	 */
	nmpt<VirtualMemoryPage> memory {};
	const auto memorySize { _mipTailSize };
	const auto memoryOffset { _mipTailOffset };

	memory = _memory->definePage(memoryOffset, memorySize);
	assert(!!memory);

	/**
	 *
	 */
	auto page = uptr<SparseTexturePage>(
		new SparseTexturePage(
			memory,
			layer_,
			math::uivec3 {},
			extent,
			_mipTailFirstLod,
			SparseTexturePageFlags { SparseTexturePageFlag::eOpaqueTail }
		)
	);

	auto* const result = page.get();
	_opaquePages.emplace_back(std::move(page));
	return result;
}

// Warning: TODO: Rewrite !?!
void SparseTexture::assureTiledPages(u32 layer_, math::uivec2 mipLevels_, math::uivec3 offset_, math::uivec3 extent_) {

	assert(offset_.zero());
	assert(extent_ == _extent);

	/**
	 * Sanitize request values
	 */
	const u32 maxTiledMipLevel { std::min(mipLevels_.max + 1u, _mipTailFirstLod) };

	/**
	 * Collect pages of targeted region
	 */
	Vector<Vector<nmpt<SparseTexturePage>>> tiledPages {};
	tiledPages.resize(maxTiledMipLevel);

	for (const auto& entry : _pages) {

		if (entry->layer() != layer_) {
			continue;
		}

		if (mipLevels_.min > entry->mipLevel() || mipLevels_.max < entry->mipLevel()) {
			continue;
		}

		tiledPages[entry->mipLevel()].emplace_back(entry.get());
	}

	/**
	 * Assure every mip has it's desired tiling pages
	 */
	for (u32 mip { mipLevels_.min }; mip < maxTiledMipLevel; ++mip) {

		math::uivec3 mipSize { _extent.x >> mip, _extent.y >> mip, 1uL };
		math::uivec3 desiredTiles { mipSize.x / _granularity.x, mipSize.y / _granularity.y, 1uL };

		if (mipSize.x > _granularity.x * desiredTiles.x) {
			++desiredTiles.x;
		}

		if (mipSize.y > _granularity.y * desiredTiles.y) {
			++desiredTiles.y;
		}

		const u32 desiredMipTiles { desiredTiles.x * desiredTiles.y * desiredTiles.z };

		if (tiledPages[mip].size() == desiredMipTiles) {
			continue;
		}

		for (u32 tz { 0uL }; tz < desiredTiles.z; ++tz) {
			for (u32 ty { 0uL }; ty < desiredTiles.y; ++ty) {
				for (u32 tx { 0uL }; tx < desiredTiles.x; ++tx) {

					const math::uivec3 tileOffset {
						tx * _granularity.x,
						ty * _granularity.y,
						tz * _granularity.z
					};
					const math::uivec3 tileExtent {
						(tx + 1uL > desiredTiles.x && mipSize.x % _granularity.x) ?
						mipSize.x % _granularity.x :
						_granularity.x,
						(ty + 1uL > desiredTiles.y && mipSize.y % _granularity.y) ?
						mipSize.y % _granularity.y :
						_granularity.y,
						(tz + 1uL > desiredTiles.z && mipSize.z % _granularity.z) ?
						mipSize.z % _granularity.z :
						_granularity.z,
					};

					auto page { makePage(layer_, mip, tileOffset, tileExtent) };
					tiledPages[mip].push_back(page);
				}
			}
		}
	}

	/**
	 * Assure every mip has it's desired opaque pages within miptail
	 */
	bool hasOpaque { false };

	for (const auto& entry : _opaquePages) {
		if (entry->layer() == layer_) {
			hasOpaque = true;
			break;
		}
	}

	if (_mipTailFirstLod < mipLevels_.max && !hasOpaque) {
		makeOpaquePage(layer_);
	}
}

void SparseTexture::selectPages(
	math::uivec2 layers_,
	math::uivec2 mipLevels_,
	math::uivec3 offset_,
	math::uivec3 extent_,
	ref<Vector<nmpt<SparseTexturePage>>> pages_
) {

	for (const auto& entry : _pages) {

		if (layers_.min > entry->layer() || entry->layer() > layers_.max) {
			continue;
		}

		if (mipLevels_.min > entry->mipLevel() || entry->mipLevel() > mipLevels_.max) {
			continue;
		}

		// TODO: Validate offset
		// TODO: Validate extent

		pages_.emplace_back(entry.get());
	}

	/**
	 * Handle mip levels which intersect the mip tail
	 */
	if (mipLevels_.max > _mipTailFirstLod) {
		for (const auto& entry : _opaquePages) {

			if (layers_.min > entry->layer() || entry->layer() > layers_.max) {
				continue;
			}

			/**
			 * While intersecting the the mip tail we can't split the resulting pages into subresource
			 *  -> No possibility for extended mip, offset or extent checks
			 */
			pages_.emplace_back(entry.get());
		}
	}
}

uptr<SparseTextureView> SparseTexture::makeView(math::uivec2 layers_, math::uivec2 mipLevels_) {

	math::uivec2 mipLevels {
		_mipLevels.min > mipLevels_.min ? _mipLevels.min : mipLevels_.min,
		_mipLevels.max < mipLevels_.max ? _mipLevels.max : mipLevels_.max,
	};

	/**
	 * Sanitize mip levels regarding mip tail region
	 *
	 * @details Extent the mip levels of the resulting view to capture the whole mip tail, cause we can't split an opaque binding
	 */
	if (mipLevels.max >= _mipTailFirstLod) {
		mipLevels.max = _mipLevels.max;
	}

	/**
	 * Assure view will be fully backed by virtual texture pages
	 */
	for (auto layer { layers_.min }; layer <= layers_.max; ++layer) {
		assureTiledPages(layer, mipLevels, {}, _extent);
	}

	Vector<nmpt<SparseTexturePage>> pages {};
	selectPages({ layers_.min, layers_.max + 1uL }, mipLevels, {}, _extent, pages);

	/**
	 * Take the time to sort the pages by it's mip level (virtual backing)
	 */
	std::ranges::sort(
		pages,
		[](nmpt<SparseTexturePage> left_, nmpt<SparseTexturePage> right_) {
			return left_->mipLevel() < right_->mipLevel();
		}
	);

	/**
	 *
	 */
	const auto view {
		new SparseTextureView(
			this,
			std::move(pages),
			layers_,
			_extent,
			_format,
			mipLevels,
			_type
		)
	};

	return std::unique_ptr<SparseTextureView>(view);
}

uptr<SparseTextureView> SparseTexture::makeSpatialView(
	math::uivec2 offset_,
	math::uivec2 extent_,
	math::uivec2 mipLevels_
) {
	::hg::todo_panic();
}

u32 SparseTexture::layers() const noexcept {
	return _layers;
}

math::uivec3::value_type SparseTexture::width() const noexcept {
	return _extent.x;
}

math::uivec3::value_type SparseTexture::height() const noexcept {
	return _extent.y;
}

math::uivec3::value_type SparseTexture::depth() const noexcept {
	return _extent.z;
}

TextureFormat SparseTexture::format() const noexcept {
	return _format;
}

math::uivec2::value_type SparseTexture::minMipLevel() const noexcept {
	return _mipLevels.min;
}

math::uivec2::value_type SparseTexture::mipLevels() const noexcept {
	return (_mipLevels.max - _mipLevels.min) + 1uL;
}

math::uivec2::value_type SparseTexture::maxMipLevel() const noexcept {
	return _mipLevels.max;
}

TextureType SparseTexture::type() const noexcept {
	return _type;
}

const decltype(SparseTexture::_granularity) SparseTexture::granularity() const noexcept {
	return _granularity;
}

const decltype(SparseTexture::_mipTailFirstLod) SparseTexture::mipTailFirstLod() const noexcept {
	return _mipTailFirstLod;
}

const decltype(SparseTexture::_mipTailSize) SparseTexture::mipTailSize() const noexcept {
	return _mipTailSize;
}

const decltype(SparseTexture::_mipTailOffset) SparseTexture::mipTailOffset() const noexcept {
	return _mipTailOffset;
}

const decltype(SparseTexture::_mipTailStride) SparseTexture::mipTailStride() const noexcept {
	return _mipTailStride;
}

cref<vk::Image> SparseTexture::vkImage() const noexcept {
	return _vkImage;
}

bool SparseTexture::load(nmpt<SparseTexturePage> page_) {

	/**
	 * Track Changes
	 */
	if (page_->flags() & SparseTexturePageFlag::eOpaqueTail) {
		_changedOpaquePages.insert(page_);
	} else {
		_changedPages.insert(page_);
	}

	/**
	 *
	 */
	return page_->load();
}

bool SparseTexture::unload(nmpt<SparseTexturePage> page_) {

	/**
	 * Track Changes
	 */
	if (page_->flags() & SparseTexturePageFlag::eOpaqueTail) {
		_changedOpaquePages.insert(page_);
	} else {
		_changedPages.insert(page_);
	}

	/**
	 *
	 */
	return page_->unload();
}

void SparseTexture::unsafe_scan_changes() {

	/**
	 *
	 */
	_changedPages.clear();
	_changedOpaquePages.clear();

	/**
	 *
	 */
	for (const auto& page : _pages) {
		_changedPages.emplace(page.get());
	}
	for (const auto& opaquePage : _opaquePages) {
		_changedOpaquePages.emplace(opaquePage.get());
	}
}

void SparseTexture::updateBindingData() {

	_opaqueBindings.clear();
	_bindings.clear();

	for (const auto& page : _changedPages) {

		// TODO: Check residential `(page->residential())`
		// TODO: Check changed memory state `(page->memoryChanged())`

		_bindings.push_back(page->vkSparseImageMemoryBind());
	}

	for (const auto& page : _changedOpaquePages) {

		// TODO: Check residential `(page->residential())`
		// TODO: Check changed memory state `(page->memoryChanged())`

		_opaqueBindings.push_back(page->vkSparseMemoryBind());
	}

	/**
	 *
	 */
	_bindData = vk::SparseImageMemoryBindInfo {
		_vkImage,
		static_cast<u32>(_bindings.size()),
		_bindings.data()
	};

	_opaqueBindData = vk::SparseImageOpaqueMemoryBindInfo {
		_vkImage,
		static_cast<u32>(_opaqueBindings.size()),
		_opaqueBindings.data()
	};

	/**
	 *
	 */
	_changedPages.clear();
	_changedOpaquePages.clear();
}

void SparseTexture::enqueueBindingSync(const ptr<CommandQueue> queue_) {

	vk::BindSparseInfo bsi {
		0,
		nullptr,
		0,
		nullptr,
		_opaqueBindData.bindCount > 0 ? 1uL : 0uL,
		&_opaqueBindData,
		_bindData.bindCount > 0 ? 1uL : 0uL,
		&_bindData,
		0,
		nullptr,
		nullptr
	};

	auto fence { queue_->device()->vkDevice().createFence(vk::FenceCreateInfo {}) };

	#ifdef _DEBUG
	auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
	assert(res == vk::Result::eSuccess);
	#else
	[[maybe_unused]] auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
	#endif

	res = queue_->device()->vkDevice().waitForFences(1uL, &fence, VK_TRUE, UINT64_MAX);
	queue_->device()->vkDevice().destroyFence(fence);

	assert(res == vk::Result::eSuccess);
}
