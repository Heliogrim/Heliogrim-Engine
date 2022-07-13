#include "VirtualTexture.hpp"

#include <ranges>
#include <Engine.Common/Make.hpp>

#include "VirtualTexturePage.hpp"
#include "VirtualTextureView.hpp"

// TODO: Remove
#include "Engine.GFX/Graphics.hpp"
#include "Engine.Session/Session.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualTexture::VirtualTexture(
    mref<uptr<VirtualMemory>> memory_,
    const u32 layer_, const math::uivec3 extent_,
    const TextureFormat format_, const math::uivec2 mipLevels_,
    const TextureType type_, vk::Image vkImage_
) :
    _memory(_STD move(memory_)),
    _layer(layer_),
    _extent(extent_),
    _format(format_),
    _mipLevels(mipLevels_),
    _type(type_),
    _vkImage(vkImage_) {}

VirtualTexture::~VirtualTexture() {
    tidy();
}

void VirtualTexture::tidy() {

    /**
     * Destroy Image before backing memory or pages
     */
    if (_vkImage) {
        Session::get()->modules().graphics()->getCurrentDevice()->vkDevice().destroyImage(_vkImage);
    }

    /**
     * Delete every virtual texture page and implicitly pages virtual memory before backing memory
     */
    for (auto& page : _pages) {

        // Unhook paged memory
        _memory->undefinePage(page->memory());

        delete page;
        page = nullptr;
    }

    _pages.clear();

    /**
     * Delete virtual memory and it's descendents
     */
    _memory.reset();
}

non_owning_rptr<VirtualTexturePage> VirtualTexture::makePage(u32 layer_, u32 mipLevel_) {

    #ifndef _DEBUG
    throw NotImplementedException();
    #endif

    assert(
        _format == TextureFormat::eR8G8B8A8Unorm ||
        _format == TextureFormat::eR32G32B32A32Sfloat ||
        _format == TextureFormat::eR16Sfloat);
    assert(_type == TextureType::e2dArray);

    #ifdef _DEBUG
    assert(_layer >= layer_);
    assert(_mipLevels.min <= mipLevel_ && mipLevel_ <= _mipLevels.max);
    #endif

    auto extent { _extent };
    for (u32 mip { 0 }; mip < mipLevel_; ++mip) {
        //extent /= 2ui32;
        extent.x >>= 2ui32;
        extent.y >>= 2ui32;
        extent.z >>= 2ui32;
    }

    /**
     *
     */
    VirtualMemoryPage* memory { nullptr };

    switch (_format) {
        case TextureFormat::eR16Sfloat: {

            const auto memorySize {
                16ui32/* Bits per Channel */
                * 1ui32/* Channels */
                * (extent.x * extent.y * extent.z)/* Dimensions */
                / 8ui32/* Bits to Bytes*/
            };

            memory = _memory->definePage(memorySize);

            break;

        }
        case TextureFormat::eR8G8B8A8Unorm: {

            const auto memorySize {
                8ui32/* Bits per Channel */
                * 4ui32/* Channels */
                * (extent.x * extent.y * extent.z)/* Dimensions */
                / 8ui32/* Bits to Bytes*/
            };

            memory = _memory->definePage(memorySize);

            break;
        }
        case TextureFormat::eR32G32B32A32Sfloat: {
            const auto memorySize {
                32ui32/* Bits per Channel */
                * 4ui32/* Channels */
                * (extent.x * extent.y * extent.z)/* Dimensions */
                / 8ui32/* Bits to Bytes*/
            };

            memory = _memory->definePage(memorySize);

            break;
        }
        default: __debugbreak();
    }

    assert(!!memory);

    /**
     *
     */
    auto* page {
        new VirtualTexturePage(
            memory,
            layer_,
            math::uivec3 {},
            extent,
            mipLevel_
        )
    };

    _pages.push_back(page);
    return page;
}

non_owning_rptr<VirtualTexturePage> VirtualTexture::makeSpatialPage(math::uivec2 offset_, math::uivec2 extent_,
    u32 mipLevel_) {
    throw NotImplementedException();
}

uptr<VirtualTextureView> VirtualTexture::makeView(u32 layer_, math::uivec2 mipLevels_) {

    const math::uivec2 mipLevels {
        _mipLevels.min > mipLevels_.min ? _mipLevels.min : mipLevels_.min,
        _mipLevels.max < mipLevels_.max ? _mipLevels.max : mipLevels_.max,
    };

    Vector<non_owning_rptr<VirtualTexturePage>> pages {};
    pages.reserve(_mipLevels.max - _mipLevels.min + 1ui32);

    for (auto* const entry : _pages) {

        if (entry->layer() != layer_) {
            continue;
        }

        if (mipLevels.min > entry->mipLevel() || mipLevels.max < entry->mipLevel()) {
            continue;
        }

        pages.push_back(entry);
    }

    /**
     * Assure view will be fully backed by virtual texture pages
     */
    for (u32 mip { mipLevels.min }; mip < mipLevels.max; ++mip) {
        auto it {
            _STD ranges::find_if(pages, [mip](non_owning_rptr<VirtualTexturePage> page_) {
                return page_->mipLevel() == mip;
            })
        };

        if (it != pages.end()) {
            continue;
        }

        /**
         * Create new pages if necessary
         */
        auto* page { makePage(layer_, mip) };
        pages.push_back(page);
    }

    /**
     * Take the time to sort the pages by it's mip level (virtual backing)
     */
    _STD ranges::sort(pages, [](non_owning_rptr<VirtualTexturePage> left_, non_owning_rptr<VirtualTexturePage> right_) {
        return left_->mipLevel() < right_->mipLevel();
    });

    /**
     *
     */
    const auto view {
        new VirtualTextureView(
            this,
            _STD move(pages),
            layer_,
            _extent,
            _format,
            mipLevels,
            _type
        )
    };

    return _STD unique_ptr<VirtualTextureView>(view);
}

uptr<VirtualTextureView> VirtualTexture::makeSpatialView(math::uivec2 offset_, math::uivec2 extent_,
    math::uivec2 mipLevels_) {
    #ifdef _DEBUG
    __debugbreak();
    #endif
    throw NotImplementedException();
}

u32 VirtualTexture::layers() const noexcept {
    return _layer;
}

math::uivec3::value_type VirtualTexture::width() const noexcept {
    return _extent.x;
}

math::uivec3::value_type VirtualTexture::height() const noexcept {
    return _extent.y;
}

math::uivec3::value_type VirtualTexture::depth() const noexcept {
    return _extent.z;
}

TextureFormat VirtualTexture::format() const noexcept {
    return _format;
}

math::uivec2::value_type VirtualTexture::minMipLevel() const noexcept {
    return _mipLevels.min;
}

math::uivec2::value_type VirtualTexture::mipLevels() const noexcept {
    return (_mipLevels.max - _mipLevels.min) + 1ui32;
}

math::uivec2::value_type VirtualTexture::maxMipLevel() const noexcept {
    return _mipLevels.max;
}

TextureType VirtualTexture::type() const noexcept {
    return _type;
}

cref<vk::Image> VirtualTexture::vkImage() const noexcept {
    return _vkImage;
}
