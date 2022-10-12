#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.GFX/Color/Color.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>

#include "../Font/Font.hpp"
#include "Engine.GFX/Texture/Texture.hpp"

namespace ember::engine::gfx::glow::ui {

    class UICommandBuffer {

    public:
        void begin();

        void end();

        void reset(const bool free_ = false);

        //private:
    public:
        Vector<u32> _runningIndexes;
        Vector<vertex> _runningVertices;

        Vector<_STD pair<u32, u32>> _imageIndices;
        Vector<sptr<Texture>> _images;
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

        void drawArc(math::vec2 pos_, const float radius_, const float from_, const float to_, cref<color> color_);

    public:
        void drawText(math::vec2 pos_, cref<string_view> text_, cref<Font> font_, const float fontSize_,
            cref<color> color_);

        void drawImage(
            math::vec2 p0_, math::vec2 uv0_,
            math::vec2 p1_, math::vec2 uv1_,
            math::vec2 p2_, math::vec2 uv2_,
            math::vec2 p3_, math::vec2 uv3_,
            sptr<Texture> image_,
            cref<color> color_
        );

        void drawImageAsync(
            math::vec2 p0_, math::vec2 uv0_,
            math::vec2 p1_, math::vec2 uv1_,
            math::vec2 p2_, math::vec2 uv2_,
            math::vec2 p3_, math::vec2 uv3_,
            sptr<Texture> image_,
            vk::Semaphore wait_,
            vk::Semaphore signal_,
            cref<color> color_
        );

    };

}
