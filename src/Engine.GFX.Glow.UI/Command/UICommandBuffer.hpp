#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.GFX/Color/Color.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>

#include "../Font/Font.hpp"
#include "Engine.GFX/Texture/ProxyTexture.hpp"

namespace ember::engine::gfx::glow::ui {

    class UICommandBuffer {

    public:
        void begin();

        void end();

        void reset(const bool free_ = false);

    public:
        Stack<math::fExtent2D> _scissorStack;

    public:
        void pushScissor(cref<math::fExtent2D> scissor_);

        void pushIntersectScissor(cref<math::fExtent2D> scissor_);

        math::fExtent2D popScissor();

    public:
        [[nodiscard]] bool scissorCull(cref<math::vec2> p_, float r_) const noexcept;

        //private:
    public:
        Vector<u32> _runningIndexes;
        Vector<uivertex> _runningVertices;

        Vector<_STD pair<u32, u32>> _scissorIndices;
        Vector<math::fExtent2D> _scissors;

        Vector<_STD pair<u32, u32>> _imageIndices;
        Vector<ProxyTexture<non_owning_rptr>> _images;
        Vector<vk::Semaphore> _imageWait;
        Vector<vk::Semaphore> _imageSignal;

    public:
        void drawLine(math::vec2 p0_, math::vec2 p1_, const float strength_);

        void drawTriangle(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_);

        void drawTriangleLine(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_,
            const float strength_);

        void drawQuad(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_);

        void drawQuadLine(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_,
            const float strength_);

        void drawRect(math::vec2 min_, math::vec2 max_, cref<color> color_);

        void drawRectLine(math::vec2 min_, math::vec2 max_, cref<color> color_, const float strength_);

        void drawArc(math::vec2 pos_, const float radius_, const float fromTheta_, const float toTheta_,
            cref<color> color_);

    public:
        void drawText(math::vec2 pos_, cref<string_view> text_, _In_ ref<Font> font_, const float fontSize_,
            cref<color> color_);

        void drawImage(
            math::vec2 p0_, math::vec2 uv0_,
            math::vec2 p1_, math::vec2 uv1_,
            math::vec2 p2_, math::vec2 uv2_,
            math::vec2 p3_, math::vec2 uv3_,
            ProxyTexture<non_owning_rptr> image_,
            cref<color> color_
        );

        void drawImageAsync(
            math::vec2 p0_, math::vec2 uv0_,
            math::vec2 p1_, math::vec2 uv1_,
            math::vec2 p2_, math::vec2 uv2_,
            math::vec2 p3_, math::vec2 uv3_,
            ProxyTexture<non_owning_rptr> image_,
            vk::Semaphore wait_,
            vk::Semaphore signal_,
            cref<color> color_
        );

    };

}
