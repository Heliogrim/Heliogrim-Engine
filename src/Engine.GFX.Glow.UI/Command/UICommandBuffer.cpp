#include "UICommandBuffer.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

void UICommandBuffer::begin() {}

void UICommandBuffer::end() {}

void UICommandBuffer::reset(const bool free_) {
    /**
     *
     */
    _runningIndexes.clear();
    _runningVertices.clear();

    _imageIndices.clear();
    _images.clear();

    _imageWait.clear();
    _imageSignal.clear();

    /**
     *
     */
    if (free_) {
        _runningIndexes.shrink_to_fit();
        _runningVertices.shrink_to_fit();

        _imageIndices.shrink_to_fit();
        _images.shrink_to_fit();

        _imageWait.shrink_to_fit();
        _imageSignal.shrink_to_fit();
    }
}

void UICommandBuffer::drawLine(math::vec2 p0_, math::vec2 p1_, const float strength_) {}

void UICommandBuffer::drawTriangle(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_) {}

void UICommandBuffer::drawTriangleLine(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_,
    const float strength_) {}

void UICommandBuffer::drawQuad(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_) {

    /**
     *
     */
    constexpr u32 cqi[] { 0ui32, 1ui32, 2ui32, 2ui32, 3ui32, 0ui32 };
    _runningIndexes.reserve(_runningIndexes.size() + (sizeof(cqi) / sizeof(u32)));

    const u32 baseIdx { static_cast<u32>(_runningVertices.size()) };
    _runningVertices.reserve(baseIdx + 4ui32);

    /**
     *
     */
    _runningIndexes.push_back(baseIdx + cqi[0]);
    _runningIndexes.push_back(baseIdx + cqi[1]);
    _runningIndexes.push_back(baseIdx + cqi[2]);
    _runningIndexes.push_back(baseIdx + cqi[3]);
    _runningIndexes.push_back(baseIdx + cqi[4]);
    _runningIndexes.push_back(baseIdx + cqi[5]);

    _runningVertices.push_back(vertex {
        math::vec3 { p0_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 {},
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p1_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 {},
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p2_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 {},
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p3_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 {},
        math::vec3 {},
        math::vec3 {}
    });
}

void UICommandBuffer::drawQuadLine(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_,
    const float strength_) {}

void UICommandBuffer::drawRect(math::vec2 min_, math::vec2 max_, cref<color> color_) {}

void UICommandBuffer::drawRectLine(math::vec2 min_, math::vec2 max_, cref<color> color_, const float strength_) {}

void UICommandBuffer::drawArc(math::vec2 pos_, const float radius_, const float from_, const float to_,
    cref<color> color_) {}

void UICommandBuffer::drawText(math::vec2 pos_, cref<string_view> text_, cref<Font> font_, const float fontSize_,
    cref<color> color_) {

    if (color_.a <= 0.F || text_.empty()) {
        return;
    }

    math::vec2 charScale { fontSize_ / font_.fontSize() };

    /**
     *
     */
    float accw { 0.F };
    for (const auto& letter : text_) {
        const auto& glyph { font_.glyph(static_cast<u32>(letter)) };
        accw += glyph.advance * charScale.x;
    }

    /**
     *
     */
    const u32 baseIdx { static_cast<u32>(_runningIndexes.size()) };
    const u32 baseVtx { static_cast<u32>(_runningVertices.size()) };

    const u32 reqIdx { static_cast<u32>(text_.size()) * 6ui32 };
    const u32 reqVtx { static_cast<u32>(text_.size()) * 4ui32 };

    _runningIndexes.reserve(baseIdx + reqIdx);
    _runningVertices.reserve(baseVtx + reqVtx);

    /**
     *
     */
    _imageIndices.push_back({ baseIdx, baseIdx + reqIdx });
    _images.push_back(font_.atlas());

    /**
     *
     */
    constexpr u32 cqi[] { 0ui32, 1ui32, 2ui32, 2ui32, 3ui32, 0ui32 };

    math::vec2 fwd { pos_ };
    for (const auto& letter : text_) {

        /**/
        const auto& glyph { font_.glyph(static_cast<u32>(letter)) };

        /**/
        const u32 cbv { static_cast<u32>(_runningVertices.size()) };

        _runningIndexes.push_back(cbv + cqi[0]);
        _runningIndexes.push_back(cbv + cqi[1]);
        _runningIndexes.push_back(cbv + cqi[2]);
        _runningIndexes.push_back(cbv + cqi[3]);
        _runningIndexes.push_back(cbv + cqi[4]);
        _runningIndexes.push_back(cbv + cqi[5]);

        _runningVertices.push_back(vertex {
            math::vec3 { fwd + glyph.xy0 * charScale, 0.F },
            math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
            math::vec3 { glyph.st0, 0.F },
            math::vec3 {},
            math::vec3 {}
        });
        _runningVertices.push_back(vertex {
            math::vec3 { fwd.x + glyph.xy1.x * charScale.x, fwd.y + glyph.xy0.y * charScale.y, 0.F },
            math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
            math::vec3 { glyph.st1.s, glyph.st0.t, 0.F },
            math::vec3 {},
            math::vec3 {}
        });
        _runningVertices.push_back(vertex {
            math::vec3 { fwd + glyph.xy1 * charScale, 0.F },
            math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
            math::vec3 { glyph.st1, 0.F },
            math::vec3 {},
            math::vec3 {}
        });
        _runningVertices.push_back(vertex {
            math::vec3 { fwd.x + glyph.xy0.x * charScale.x, fwd.y + glyph.xy1.y * charScale.y, 0.F },
            math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
            math::vec3 { glyph.st0.s, glyph.st1.t, 0.F },
            math::vec3 {},
            math::vec3 {}
        });

        /**/
        fwd.x += glyph.advance * charScale.x;
    }
}

void UICommandBuffer::drawImage(
    math::vec2 p0_, math::vec2 uv0_,
    math::vec2 p1_, math::vec2 uv1_,
    math::vec2 p2_, math::vec2 uv2_,
    math::vec2 p3_, math::vec2 uv3_,
    sptr<Texture> image_,
    cref<color> color_
) {
    drawImageAsync(p0_, uv0_, p1_, uv1_, p2_, uv2_, p3_, uv3_, image_, VK_NULL_HANDLE, VK_NULL_HANDLE, color_);
}

void UICommandBuffer::drawImageAsync(
    math::vec2 p0_, math::vec2 uv0_,
    math::vec2 p1_, math::vec2 uv1_,
    math::vec2 p2_, math::vec2 uv2_,
    math::vec2 p3_, math::vec2 uv3_,
    sptr<Texture> image_,
    vk::Semaphore wait_,
    vk::Semaphore signal_,
    cref<color> color_
) {

    /**
     *
     */
    constexpr u32 cqi[] { 0ui32, 1ui32, 2ui32, 2ui32, 3ui32, 0ui32 };
    const u32 baseIdx { static_cast<u32>(_runningIndexes.size()) };
    _runningIndexes.reserve(_runningIndexes.size() + (sizeof(cqi) / sizeof(u32)));

    const u32 baseVtx { static_cast<u32>(_runningVertices.size()) };
    _runningVertices.reserve(baseVtx + 4ui32);

    /**
     *
     */
    _runningIndexes.push_back(baseVtx + cqi[0]);
    _runningIndexes.push_back(baseVtx + cqi[1]);
    _runningIndexes.push_back(baseVtx + cqi[2]);
    _runningIndexes.push_back(baseVtx + cqi[3]);
    _runningIndexes.push_back(baseVtx + cqi[4]);
    _runningIndexes.push_back(baseVtx + cqi[5]);

    _runningVertices.push_back(vertex {
        math::vec3 { p0_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 { uv0_, 0.F },
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p1_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 { uv1_, 0.F },
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p2_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 { uv2_, 0.F },
        math::vec3 {},
        math::vec3 {}
    });
    _runningVertices.push_back(vertex {
        math::vec3 { p3_, 0.F },
        math::vec3_t<u8> { static_cast<u8>(color_.r), static_cast<u8>(color_.g), static_cast<u8>(color_.b) },
        math::vec3 { uv3_, 0.F },
        math::vec3 {},
        math::vec3 {}
    });

    /**
     *
     */
    _imageIndices.push_back({ baseIdx, baseIdx + 6 });
    _images.push_back(image_);

    if (wait_) {
        _imageWait.push_back(wait_);
    }

    if (signal_) {
        _imageSignal.push_back(signal_);
    }
}
