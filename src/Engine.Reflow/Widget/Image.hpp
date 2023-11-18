#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Widget.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"

namespace hg::engine::reflow {
    class Image :
        public Widget {
    public:
        using this_type = Image;

    public:
        Image();

        ~Image() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        struct Attributes {
            Attribute<ReflowUnit> minWidth;
            Attribute<ReflowUnit> width;
            Attribute<ReflowUnit> maxWidth;

            Attribute<ReflowUnit> minHeight;
            Attribute<ReflowUnit> height;
            Attribute<ReflowUnit> maxHeight;

            Attribute<engine::color> tint;

            Attribute<nmpt<gfx::TextureLikeObject>> image;
        } attr;

    private:
        _STD array<math::vec2, 4> _uvs;
        ptr<void> _imageResource;

    public:
        void setImage(cref<nmpt<gfx::TextureLikeObject>> image_, ptr<void> resource_ = nullptr);

    private:
        NullChildren _children;

    public:
        const ptr<const NullChildren> children() const override;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
