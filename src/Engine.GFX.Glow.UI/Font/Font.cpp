#include "Font.hpp"

#include "Engine.GFX/Command/CommandPool.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Font::Font():
    _fontSize(16),
    _fallback(),
    _first(0),
    _glyphCount(0) {}

Font::~Font() noexcept = default;

cref<string> Font::name() const noexcept {
    return _name;
}

bool Font::isBitmap() const noexcept {
    return true;
}

u32 Font::width() const noexcept {
    return _extent.x;
}

u32 Font::height() const noexcept {
    return _extent.y;
}

float Font::fontSize() const noexcept {
    return _fontSize;
}

cref<FontGlyph> Font::glyph(const u32 letter_) const noexcept {

    if (letter_ < _first || (_first + _glyphCount) <= letter_) {
        return _fallback;
    }

    return _glyphs[letter_ - _first];
}

cref<sptr<engine::gfx::Texture>> Font::atlas() const noexcept {
    return _atlas;
}

#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include "stb_font_consolas_24_latin1.inl"

Font engine::gfx::glow::ui::testLoadDefaultFont(cref<sptr<Device>> device_) {

    /**
     *
     */
    constexpr math::uivec2 fontExtent {
        STB_FONT_consolas_24_latin1_BITMAP_WIDTH,
        STB_FONT_consolas_24_latin1_BITMAP_WIDTH
    };

    static stb_fontchar stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];
    static unsigned char font24px[fontExtent.x][fontExtent.y];
    stb_font_consolas_24_latin1(stbFontData, font24px, fontExtent.y);

    constexpr u64 memorySize { fontExtent.x * fontExtent.y * sizeof(char) };

    /**
     *
     */
    const auto* factory { engine::gfx::TextureFactory::get() };
    auto atlas = factory->build(TextureBuildPayload {
        math::uivec3 { fontExtent, 1ui32 },
        TextureFormat::eR8Unorm,
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });
    factory->buildView(atlas);

    /**
     *
     */
    auto pool = device_->graphicsQueue()->pool();
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

    /**
     *
     */
    Buffer stage {
        nullptr,
        nullptr,
        device_->vkDevice(),
        memorySize,
        vk::BufferUsageFlagBits::eTransferSrc
    };

    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags {},
        stage.size,
        stage.usageFlags,
        vk::SharingMode::eExclusive,
        0ui32,
        nullptr
    };

    stage.buffer = device_->vkDevice().createBuffer(bci);
    assert(stage.buffer);

    [[maybe_unused]] const auto result {
        memory::allocate(device_->allocator(), device_, stage.buffer,
            MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent,
            stage.memory)
    };
    stage.bind();

    /**
     *
     */
    stage.write<char>(&font24px[0][0], fontExtent.x * fontExtent.y);

    /**
     *
     */
    atlas.buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;
    cmd.copyBufferToImage(stage, atlas.buffer(), vk::BufferImageCopy {
        0ui64,
        0ui32,
        0ui32,
        vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0ui32, 0ui32, 1ui32 },
        vk::Offset3D {},
        vk::Extent3D { fontExtent.x, fontExtent.y, 1ui32 }
    });

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
        0, nullptr, 0, nullptr, static_cast<u32>(postBarrier.size()), postBarrier.data());

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
    atlas.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    /**
     *
     */
    stage.destroy();

    /**
     *
     */
    Font font {};
    font._name = "Consolas Latin1"sv;
    font._atlas = make_sptr<Texture>(_STD move(atlas));
    font._extent = fontExtent;
    font._fontSize = 24.F;
    font._first = STB_FONT_consolas_24_latin1_FIRST_CHAR;
    font._glyphCount = STB_FONT_consolas_24_latin1_NUM_CHARS;

    for (u32 idx { 0ui32 }; idx < font._glyphCount; ++idx) {
        const auto& data { stbFontData[idx] };

        font._glyphs.push_back(FontGlyph {
            math::vec2 { data.s0f, data.t0f },
            math::vec2 { data.s1f, data.t1f },
            math::vec2 { data.x0f, data.y0f },
            math::vec2 { data.x1f, data.y1f },
            data.advance
        });
    }

    return font;
}
