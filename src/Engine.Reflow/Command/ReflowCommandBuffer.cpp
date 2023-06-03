#include "ReflowCommandBuffer.hpp"

#include <Engine.GFX/Texture/TextureView.hpp>

using namespace hg::engine::reflow;
using namespace hg::engine::gfx;
using namespace hg;

void ReflowCommandBuffer::begin() {}

void ReflowCommandBuffer::end() {}

void ReflowCommandBuffer::reset(const bool free_) {

    /**
     *
     */
    while (not _scissorStack.empty()) {
        _scissorStack.pop();
    }
    _scissorIndices.clear();
    _scissors.clear();

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
        _scissorIndices.shrink_to_fit();
        _scissors.shrink_to_fit();

        _runningIndexes.shrink_to_fit();
        _runningVertices.shrink_to_fit();

        _imageIndices.shrink_to_fit();
        _images.shrink_to_fit();

        _imageWait.shrink_to_fit();
        _imageSignal.shrink_to_fit();
    }
}

void ReflowCommandBuffer::pushScissor(cref<math::fExtent2D> scissor_) {

    const u32 lastRunningIdx { static_cast<u32>(_runningIndexes.size()) };

    const bool usedPrevScissor { not _scissors.empty() && _scissorIndices.back().first != lastRunningIdx };

    if (usedPrevScissor) {
        _scissorIndices.back().second = lastRunningIdx;

        _scissorIndices.push_back({ lastRunningIdx, 0ui32 });
        _scissors.push_back(scissor_);

    } else if (not _scissors.empty()) {

        //_scissorIndices.back().first = lastRunningIdx;
        _scissors.back() = scissor_;

    } else {

        _scissorIndices.push_back({ lastRunningIdx, 0ui32 });
        _scissors.push_back(scissor_);
    }

    assert(_scissors.size() == _scissorIndices.size());
    _scissorStack.push(scissor_);
}

void ReflowCommandBuffer::pushIntersectScissor(cref<math::fExtent2D> scissor_) {

    const auto hasPrevScissor { not _scissorStack.empty() };
    auto scissor { scissor_ };

    if (hasPrevScissor) {
        const auto& prev { _scissorStack.top() };

        const math::vec2 outerMin { prev.offsetX, prev.offsetY };
        const math::vec2 outerMax { prev.offsetX + prev.width, prev.offsetY + prev.height };

        const math::vec2 innerMin { scissor_.offsetX, scissor_.offsetY };
        const math::vec2 innerMax { scissor_.offsetX + scissor_.width, scissor_.offsetY + scissor_.height };

        const math::vec2 min { math::compMax<float>(outerMin, innerMin) };
        const math::vec2 max { math::compMin<float>(outerMax, innerMax) };

        if (min.x > max.x || min.y > max.y) {
            scissor.width = 0;
            scissor.height = 0;
            scissor.offsetX = 0;
            scissor.offsetY = 0;

        } else {
            scissor.width = max.x - min.x;
            scissor.height = max.y - min.y;
            scissor.offsetX = min.x;
            scissor.offsetY = min.y;
        }
    }

    pushScissor(scissor);
}

math::fExtent2D ReflowCommandBuffer::popScissor() {

    const u32 lastRunningIdx { static_cast<u32>(_runningIndexes.size()) };

    if (not _scissorStack.empty()) {
        _scissorIndices.back().second = lastRunningIdx;
    }

    const auto top { _scissorStack.top() };
    _scissorStack.pop();

    if (not _scissorStack.empty()) {

        if (_scissorIndices.back().first == lastRunningIdx) {
            _scissors.back() = _scissorStack.top();

        } else {
            _scissorIndices.push_back({ lastRunningIdx, 0ui32 });
            _scissors.push_back(_scissorStack.top());
        }
    }

    return top;
}

bool ReflowCommandBuffer::scissorCull(cref<math::vec2> p_, float r_) const noexcept {

    if (_scissorStack.empty()) {
        return false;
    }

    const auto& scissor { _scissorStack.top() };

    if (p_.x + r_ <= scissor.offsetX || p_.x - r_ > scissor.offsetX + scissor.width) {
        return true;
    }

    if (p_.y + r_ <= scissor.offsetY || p_.y - r_ > scissor.offsetY + scissor.height) {
        return true;
    }

    return false;
}

bool ReflowCommandBuffer::scissorCull(cref<math::vec2> off_, cref<math::vec2> size_) const noexcept {

    if (_scissorStack.empty()) {
        return false;
    }

    const auto& scissor { _scissorStack.top() };

    const math::vec2 rectMax { off_ + size_ };
    if (rectMax.x < scissor.offsetX || rectMax.y < scissor.offsetY) {
        return true;
    }

    const math::vec2 scissorMax { scissor.offsetX + scissor.width, scissor.offsetY + scissor.height };
    if (scissorMax.x < off_.x || scissorMax.y < off_.y) {
        return true;
    }

    return false;
}

bool ReflowCommandBuffer::scissorCull(cref<math::fExtent2D> rect_) const noexcept {

    if (_scissorStack.empty()) {
        return false;
    }

    const auto& scissor { _scissorStack.top() };

    const math::vec2 rectMax { rect_.offsetX + rect_.width, rect_.offsetY + rect_.height };
    if (rectMax.x < scissor.offsetX || rectMax.y < scissor.offsetY) {
        return true;
    }

    const math::vec2 scissorMax { scissor.offsetX + scissor.width, scissor.offsetY + scissor.height };
    if (scissorMax.x < rect_.offsetX || scissorMax.y < rect_.offsetY) {
        return true;
    }

    return false;
}

void ReflowCommandBuffer::drawLine(math::vec2 p0_, math::vec2 p1_, const float strength_) {

    color strokeColor { 255.F, 255.F, 255.F, 255.F };

    const auto hs = strength_ / 2.F;

    const auto mainAxis = p1_ - p0_;
    const auto ortho = math::vec2 { mainAxis.y, -mainAxis.x }.normalize();

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

    _runningVertices.push_back(
        uivertex {
            math::vec2 { p0_ + ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p1_ + ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p1_ - ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p0_ - ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}
        }
    );

}

void ReflowCommandBuffer::drawTriangle(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_) {}

void ReflowCommandBuffer::drawTriangleLine(
    math::vec2 p0_,
    math::vec2 p1_,
    math::vec2 p2_,
    cref<color> color_,
    const float strength_
) {}

void ReflowCommandBuffer::drawQuad(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_) {

    if (color_.a <= 0.F) {
        return;
    }

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

    _runningVertices.push_back(
        uivertex {
            math::vec2 { p0_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p1_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p2_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 {}
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p3_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 {}
        }
    );
}

void ReflowCommandBuffer::drawQuadLine(
    math::vec2 p0_,
    math::vec2 p1_,
    math::vec2 p2_,
    math::vec2 p3_,
    cref<color> color_,
    const float strength_
) {}

void ReflowCommandBuffer::drawRect(math::vec2 min_, math::vec2 max_, cref<color> color_) {}

void ReflowCommandBuffer::drawRectLine(math::vec2 min_, math::vec2 max_, cref<color> color_, const float strength_) {}

void ReflowCommandBuffer::drawArc(
    math::vec2 pos_,
    const float radius_,
    const float fromTheta_,
    const float toTheta_,
    cref<color> color_
) {

    if (color_.a <= 0.F) {
        return;
    }

    /* Surface :: 2*PI*r */
    /* Radians :: 2*PI */
    /* Splice  :: (tt - ft) */
    /* Result  :: [2*PI*r] * [(tt-ft)/(2*PI)] */
    /*         :: [r] * [(tt-ft)] */

    u32 segments { MIN(static_cast<u32>(radius_ * (toTheta_ - fromTheta_)), 11ui32) };

    // Enforce odd segment count to prevent rounding / flattening of corner tip
    if (not segments & 0x1) {
        ++segments;
    }

    const float dtPs { (toTheta_ - fromTheta_) / static_cast<float>(segments) };

    const u32 reqVtx { 1ui32 + segments + 1ui32 };
    const u32 reqIdx { segments * 3ui32 };

    /**
     *
     */
    const u32 baseIdx { static_cast<u32>(_runningIndexes.size()) };
    const u32 baseVtx { static_cast<u32>(_runningVertices.size()) };

    _runningIndexes.reserve(baseIdx + reqIdx);
    _runningVertices.reserve(baseVtx + reqVtx);

    /**/
    _runningVertices.push_back(
        uivertex {
            math::vec2 { pos_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 {}
        }
    );

    /**/
    for (u32 seg { 0ui32 }; seg <= segments; ++seg) {
        float theta { fromTheta_ + static_cast<float>(seg) * dtPs };
        theta = MIN(theta, toTheta_);

        const math::vec2 nd { _STD cosf(theta), _STD sinf(theta) };

        _runningVertices.push_back(
            uivertex {
                math::vec2 { pos_ + nd * radius_ },
                math::vec4_t<u8> {
                    static_cast<u8>(color_.r),
                    static_cast<u8>(color_.g),
                    static_cast<u8>(color_.b),
                    static_cast<u8>(color_.a)
                },
                math::vec3 {}
            }
        );
    }

    /**/
    for (u32 seg { 0ui32 }; seg < segments; ++seg) {
        _runningIndexes.push_back(baseVtx);
        _runningIndexes.push_back(baseVtx + seg + 1ui32);
        _runningIndexes.push_back(baseVtx + seg + 2ui32);
    }
}

void ReflowCommandBuffer::drawText(
    math::vec2 pos_,
    cref<string_view> text_,
    ref<Font> font_,
    const float fontSize_,
    cref<color> color_
) {

    if (color_.a <= 0.F || text_.empty()) {
        return;
    }

    const auto fss { font_.nextFontSize(static_cast<u32>(fontSize_)) };// Next (native) Supported Size
    math::vec2 charScale { fontSize_ / static_cast<float>(fss) };

    /**
     *
     */
    float accw { 0.F };
    for (const auto& letter : text_) {
        const auto& glyph { font_.glyph(static_cast<u32>(letter), fss) };

        // TODO: Cleanup <=> Command Invocation should not be required to validate draw command
        if (glyph == nullptr) {
            continue;
        }

        accw += glyph->_advance * charScale.x;
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
    auto atlas { font_.atlas().get() };
    ProxyTexture<non_owning_rptr> proxy { _STD move(atlas) };
    _images.push_back(_STD move(proxy));

    /**
     *
     */
    constexpr u32 cqi[] { 0ui32, 1ui32, 2ui32, 2ui32, 3ui32, 0ui32 };

    math::vec2 fwd { pos_ + math::vec2 { 0.F, -0.25F * static_cast<float>(fss) * charScale.y } };
    fwd.x = _STD floorf(fwd.x);
    fwd.y = _STD floorf(fwd.y);

    for (const auto& letter : text_) {

        /**/
        const auto& glyph { font_.glyph(static_cast<u32>(letter), fss) };

        // TODO: Cleanup <=> Command Invocation should not be required to validate draw command
        if (glyph == nullptr) {
            continue;
        }

        /**/
        const u32 cbv { static_cast<u32>(_runningVertices.size()) };

        _runningIndexes.push_back(cbv + cqi[0]);
        _runningIndexes.push_back(cbv + cqi[1]);
        _runningIndexes.push_back(cbv + cqi[2]);
        _runningIndexes.push_back(cbv + cqi[3]);
        _runningIndexes.push_back(cbv + cqi[4]);
        _runningIndexes.push_back(cbv + cqi[5]);

        const float gx { fwd.x + (static_cast<float>(glyph->_bearing.x) * charScale.x) };
        const float gy { fwd.y + fontSize_ - (static_cast<float>(glyph->_bearing.y) * charScale.y) };
        const float gw { static_cast<float>(glyph->_size.x) * charScale.x };
        const float gh { static_cast<float>(glyph->_size.y) * charScale.y };

        _runningVertices.push_back(
            uivertex {
                math::vec2 {
                    gx,
                    gy
                },
                math::vec4_t<u8> {
                    static_cast<u8>(color_.r),
                    static_cast<u8>(color_.g),
                    static_cast<u8>(color_.b),
                    static_cast<u8>(color_.a)
                },
                math::vec3 { glyph->_minSt, 0.F }
            }
        );
        _runningVertices.push_back(
            uivertex {
                math::vec2 {
                    gx + gw,
                    gy
                },
                math::vec4_t<u8> {
                    static_cast<u8>(color_.r),
                    static_cast<u8>(color_.g),
                    static_cast<u8>(color_.b),
                    static_cast<u8>(color_.a)
                },
                math::vec3 { glyph->_maxSt.s, glyph->_minSt.t, 0.F }
            }
        );
        _runningVertices.push_back(
            uivertex {
                math::vec2 {
                    gx + gw,
                    gy + gh
                },
                math::vec4_t<u8> {
                    static_cast<u8>(color_.r),
                    static_cast<u8>(color_.g),
                    static_cast<u8>(color_.b),
                    static_cast<u8>(color_.a)
                },
                math::vec3 { glyph->_maxSt, 0.F }
            }
        );
        _runningVertices.push_back(
            uivertex {
                math::vec2 {
                    gx,
                    gy + gh
                },
                math::vec4_t<u8> {
                    static_cast<u8>(color_.r),
                    static_cast<u8>(color_.g),
                    static_cast<u8>(color_.b),
                    static_cast<u8>(color_.a)
                },
                math::vec3 { glyph->_minSt.s, glyph->_maxSt.t, 0.F }
            }
        );

        /**/
        fwd.x += glyph->_advance * charScale.x;
    }
}

void ReflowCommandBuffer::drawImage(
    math::vec2 p0_,
    math::vec2 uv0_,
    math::vec2 p1_,
    math::vec2 uv1_,
    math::vec2 p2_,
    math::vec2 uv2_,
    math::vec2 p3_,
    math::vec2 uv3_,
    ProxyTexture<non_owning_rptr> image_,
    cref<color> color_
) {
    drawImageAsync(
        p0_,
        uv0_,
        p1_,
        uv1_,
        p2_,
        uv2_,
        p3_,
        uv3_,
        _STD move(image_),
        nullptr,
        nullptr,
        color_
    );
}

void ReflowCommandBuffer::drawImageAsync(
    math::vec2 p0_,
    math::vec2 uv0_,
    math::vec2 p1_,
    math::vec2 uv1_,
    math::vec2 p2_,
    math::vec2 uv2_,
    math::vec2 p3_,
    math::vec2 uv3_,
    ProxyTexture<non_owning_rptr> image_,
    _::VkSemaphore wait_,
    _::VkSemaphore signal_,
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

    _runningVertices.push_back(
        uivertex {
            math::vec2 { p0_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 { uv0_, 0.F }
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p1_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 { uv1_, 0.F }
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p2_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 { uv2_, 0.F }
        }
    );
    _runningVertices.push_back(
        uivertex {
            math::vec2 { p3_ },
            math::vec4_t<u8> {
                static_cast<u8>(color_.r),
                static_cast<u8>(color_.g),
                static_cast<u8>(color_.b),
                static_cast<u8>(color_.a)
            },
            math::vec3 { uv3_, 0.F }
        }
    );

    /**
     *
     */
    _imageIndices.push_back({ baseIdx, baseIdx + 6 });
    _images.push_back(_STD move(image_));

    if (wait_) {
        _imageWait.push_back(wait_);
    }

    if (signal_) {
        _imageSignal.push_back(signal_);
    }
}
