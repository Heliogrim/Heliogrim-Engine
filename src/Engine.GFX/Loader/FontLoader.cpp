#include "FontLoader.hpp"

#include <Engine.Common/Wrapper.hpp>

#include "../Cache/GlobalCacheCtrl.hpp"
#include "../Cache/GlobalResourceCache.hpp"
#include "../Resource/FontResource.hpp"
#include <Engine.Reflow/Font/GlyphRanges.hpp>

#if TRUE
#include <freetype/ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>

FT_Library freeTypeLibrary {};
FT_Face cascadiaCodeFace {};

void prepare_freetype_library() {

    auto error { FT_Init_FreeType(&freeTypeLibrary) };
    if (error) {
        __debugbreak();
    }

    const _STD filesystem::path srcFile { R"(\resources\imports\ttf\CascadiaCode.ttf)" };
    const auto cwd { _STD filesystem::current_path() };

    const _STD filesystem::path srcPath { cwd.string() + srcFile.string() };

    // error = FT_New_Face(freeTypeLibrary, srcPath.string().c_str(), -1, &cascadiaCodeFace);
    error = FT_New_Face(freeTypeLibrary, srcPath.string().c_str(), 0, &cascadiaCodeFace);

    if (error == FT_Err_Unknown_File_Format) {
        __debugbreak();
    } else if (error) {
        __debugbreak();
    }

    FT_Set_Pixel_Sizes(cascadiaCodeFace, 0, 16);
}

void cleanup_freetype_library() {
    FT_Done_Face(cascadiaCodeFace);
    FT_Done_FreeType(freeTypeLibrary);
}
#endif

using namespace ember::engine::gfx;
using namespace ember;

static sptr<engine::reflow::Font> __tmp_cache {};

static engine::res::LoaderOptions<engine::assets::Font> DefaultFontOptions {
    { engine::reflow::BasicLatin, engine::reflow::Latin1Supplement },
    {},
    //
    { 12ui32, 16ui32, 24ui32 }
};

constexpr math::uivec2 font_texture_padding { 1ui32 };

FontLoader::FontLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    Loader(),
    _cacheCtrl(cache_) {

    prepare_freetype_library();
}

FontLoader::~FontLoader() {
    cleanup_freetype_library();

    __tmp_cache.reset();
}

FontLoader::result_type FontLoader::operator()(
    const ptr<assets::Font> asset_,
    options_type options_
) {
    auto* res { make_ptr<FontResource>() };

    /**/
    if (!options_) {
        options_ = &DefaultFontOptions;
    }
    /**/

    if (!__tmp_cache) {
        __tmp_cache = sptr<reflow::Font>(static_cast<ptr<reflow::Font>>(__test_load(options_)));
    }

    res->_fontData = __tmp_cache.get();
    return res;
}

#if TRUE
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include "../Device/Device.hpp"
#endif

math::uivec2 __getFontExtent(ptr<::ember::engine::res::LoaderOptions<::engine::assets::Font>> options_) {

    const auto& face { cascadiaCodeFace };

    math::uivec2 extent {};
    for (const auto& fontSize : options_->fontSizes) {

        /**/
        FT_Set_Pixel_Sizes(face, 0, fontSize);
        /**/

        for (const auto& single : options_->glyphs) {
            /**/
            auto glyphIndex { FT_Get_Char_Index(face, single) };

            if (glyphIndex == 0) {
                continue;
            }

            /**/
            auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

            if (error) {
                continue;
            }

            /**/
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

            if (error) {
                continue;
            }

            /**/
            //extent.x += face->glyph->advance.x >> 6;
            //extent.x += face->glyph->advance.x;
            extent.x += face->glyph->bitmap.width + font_texture_padding.x;
            //extent.y += face->glyph->advance.y >> 6;
            extent.y = MAX(extent.y, face->glyph->bitmap.rows + font_texture_padding.y);
        }

        for (const auto& range : options_->ranges) {
            for (engine::reflow::GlyphCode c { range.begin }; c < range.end; ++c) {

                /**/
                auto glyphIndex { FT_Get_Char_Index(face, c) };

                if (glyphIndex == 0) {
                    continue;
                }

                /**/
                auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

                if (error) {
                    continue;
                }

                /**/
                error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

                if (error) {
                    continue;
                }

                /**/
                //extent.x += face->glyph->advance.x >> 6;
                //extent.x += face->glyph->advance.x;
                extent.x += face->glyph->bitmap.width + font_texture_padding.x;
                //extent.y += face->glyph->advance.y >> 6;
                extent.y = MAX(extent.y, face->glyph->bitmap.rows + font_texture_padding.y);
            }
        }
    }

    return extent;
}

void __writeToMemory(cref<FT_GlyphSlot> slot_, cref<math::uivec2> ext_, cref<math::uivec2> off_, ptr<void> dst_) {

    u8* dst { static_cast<ptr<u8>>(dst_) };
    auto* cur { dst };

    cur += off_.y * ext_.x * 4ui32;
    cur += off_.x * 4ui32;

    const auto wrapDiff { ext_.x - slot_->bitmap.width };

    const auto gw { slot_->bitmap.width };
    const auto gh { slot_->bitmap.rows };

    auto* src { slot_->bitmap.buffer };
    auto* line { src };

    for (u32 y { 0 }; y < gh; ++y) {

        src = line;
        for (u32 x { 0 }; x < gw; ++x) {
            const auto val { *src };
            ++src;

            *cur = 0xFFui8;
            ++cur;
            *cur = 0xFFui8;
            ++cur;
            *cur = 0xFFui8;
            ++cur;
            *cur = val;
            ++cur;
        }

        line += slot_->bitmap.pitch;
        cur += wrapDiff * 4;
    }
}

void __loadFontToTexture(
    const ptr<engine::reflow::Font> font_,
    ptr<::ember::engine::res::LoaderOptions<::ember::engine::assets::Font>> options_,
    cref<math::uivec2> report_,
    ptr<void> dst_
) {

    const auto& face { cascadiaCodeFace };

    const auto& atlas { font_->_atlas };
    const math::uivec2 atlasExt { atlas->width(), atlas->height() };

    math::uivec2 step { 0, report_.y };
    math::uivec2 fwd { 0 };

    u32 curSize { _STD numeric_limits<u32>::max() };
    for (u32 i { 0 }; i < options_->fontSizes.size(); ++i) {

        u32 nextSize { 0ui32 };
        for (const auto& entry : options_->fontSizes) {
            if (entry < curSize && entry > nextSize) {
                nextSize = entry;
            }
        }
        curSize = nextSize;

        /**/
        FT_Set_Pixel_Sizes(face, 0, nextSize);

        /**/
        font_->_sizes.push_back(nextSize);

        for (const auto& range : options_->ranges) {
            for (engine::reflow::GlyphCode c { range.begin }; c < range.end; ++c) {

                /**/
                auto glyphIndex { FT_Get_Char_Index(face, c) };

                if (glyphIndex == 0) {
                    continue;
                }

                /**/
                auto error { FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT) };

                if (error) {
                    continue;
                }

                /**/
                error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

                if (error) {
                    continue;
                }

                /**/
                const auto nfwd { fwd.x + /*face->glyph->advance.x*/face->glyph->bitmap.width };
                if (nfwd > atlas->width()) {
                    fwd.x = 0ui32;
                    fwd.y += step.y + font_texture_padding.y;
                }

                /**/
                __writeToMemory(face->glyph, atlasExt, fwd, dst_);

                /**/
                const math::vec2 viewMinSt {
                    static_cast<float>(fwd.x) / static_cast<float>(atlasExt.x),
                    static_cast<float>(fwd.y) / static_cast<float>(atlasExt.y)
                };
                const math::vec2 viewMaxSt {
                    static_cast<float>(fwd.x + face->glyph->bitmap.width) / static_cast<float>(atlasExt.x),
                    static_cast<float>(fwd.y + face->glyph->bitmap.rows) / static_cast<float>(atlasExt.y)
                };

                /**/
                font_->_glyphs.insert_or_assign(font_->encodeKey(c, curSize), new engine::reflow::FontGlyph {
                    math::uivec2 { face->glyph->bitmap.width, face->glyph->bitmap.rows },
                    math::ivec2 { face->glyph->bitmap_left, face->glyph->bitmap_top },
                    static_cast<float>(face->glyph->advance.x >> 6),
                    viewMinSt,
                    viewMaxSt
                });

                /**/
                //fwd.x += face->glyph->advance.x >> 6;
                fwd.x += face->glyph->bitmap.width + font_texture_padding.x;
            }
        }
    }

}

ptr<void> FontLoader::__test_load(ptr<::ember::engine::res::LoaderOptions<::ember::engine::assets::Font>> options_) {

    const auto& device { _cacheCtrl->cache()->device() };

    /**
     *
     */
    math::uivec2 repExtent { __getFontExtent(options_) };
    math::uivec2 reqExtent { repExtent };

    /**/
    if (repExtent.x > 512) {

        const auto coverSize { repExtent.x * repExtent.y };
        const auto expCoverSize { static_cast<float>(coverSize) * 1.2F };
        const auto expCoverSqrf { _STD sqrtf(expCoverSize) };

        u32 nextDim { 0x1 };
        while (nextDim < expCoverSqrf) {
            nextDim <<= 1;
        }

        reqExtent.x = nextDim;
        reqExtent.y = nextDim;
    }
    /**/

    /**
     *
     */
    const auto* factory { engine::gfx::TextureFactory::get() };
    auto atlas = factory->build(TextureBuildPayload {
        math::uivec3 { reqExtent, 1ui32 },
        TextureFormat::eR8G8B8A8Unorm,
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });
    factory->buildView(atlas);

    /**/
    Buffer buffer {
        nullptr,
        nullptr,
        device->vkDevice(),
        sizeof(u8) * reqExtent.x * reqExtent.y * 4ui32,
        vk::BufferUsageFlagBits::eTransferSrc
    };

    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        buffer.size,
        buffer.usageFlags,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    buffer.buffer = device->vkDevice().createBuffer(bci);
    assert(buffer.buffer);

    [[maybe_unused]] auto allocRes {
        memory::allocate(device->allocator(), device, buffer.buffer,
            MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent, buffer.memory)
    };
    buffer.bind();

    /**
     *
     */
    auto pool = device->graphicsQueue()->pool();
    pool->lck().acquire();
    auto cmd { pool->make() };

    cmd.begin();

    Vector<vk::ImageMemoryBarrier> preBarrier {};
    preBarrier.push_back({
        vk::AccessFlags {},
        vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        atlas.buffer().image(),
        vk::ImageSubresourceRange {
            vk::ImageAspectFlagBits::eColor,
            0,
            atlas.mipLevels(),
            0,
            atlas.layer()
        }
    });

    cmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
        0, nullptr, 0, nullptr, static_cast<u32>(preBarrier.size()), preBarrier.data());

    /**/

    auto* font { make_ptr<reflow::Font>() };
    font->_name = "Cascadia Code"sv;
    font->_atlas = make_sptr<Texture>(_STD move(atlas));
    font->_extent = reqExtent;
    font->_fontSize = 16ui32;
    font->_glyphCount = 0;

    // Warning: !!Important!!
    font->_ftFace = &cascadiaCodeFace;

    /**/

    buffer.map();
    __loadFontToTexture(font, options_, repExtent, buffer.memory->mapping);
    buffer.unmap();

    /**/

    font->_atlas->buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;
    cmd.copyBufferToImage(buffer, font->_atlas->buffer(), vk::BufferImageCopy {
        0ui64,
        0,
        0,
        vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0ui32, 0ui32, 1ui32 },
        vk::Offset3D {},
        vk::Extent3D { font->_atlas->width(), font->_atlas->height(), 1ui32 }
    });

    /**/

    /**
     *
     */
    Vector<vk::ImageMemoryBarrier> postBarrier {};
    postBarrier.push_back({
        vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlags {},
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        font->_atlas->buffer().image(),
        vk::ImageSubresourceRange {
            vk::ImageAspectFlagBits::eColor,
            0,
            font->_atlas->mipLevels(),
            0,
            font->_atlas->layer()
        }
    });

    cmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
        0, nullptr, 0, nullptr, static_cast<u32>(postBarrier.size()), postBarrier.data());

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
    atlas.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    buffer.destroy();

    return font;
}
