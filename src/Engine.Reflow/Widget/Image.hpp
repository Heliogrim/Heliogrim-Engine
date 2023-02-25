#pragma once

#include <Engine.GFX/Texture/ProxyTexture.hpp>

#include "Widget.hpp"
#include "../Style/__fwd.hpp"
#include "../Style/StyleSheet.hpp"

namespace ember::engine::reflow {
    class Image :
        public Widget {
    public:
        using this_type = Image;

    public:
        Image(mref<sptr<BoundStyleSheet>> style_);

        ~Image() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        sptr<BoundStyleSheet> _style;
        StyleSheet _computedStyle;

    private:
        sptr<gfx::ProxyTexture<non_owning_rptr>> _image;
        _STD array<math::vec2, 4> _uvs;

        ptr<void> _imageResource;

    public:
        void setImage(cref<decltype(_image)> image_, ptr<void> resource_ = nullptr);

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    private:
        math::vec2 _innerSize;
        math::vec2 _screenOff;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;
    };
}
