#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow/Widget/Box.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

namespace ember::editor::ui {

    class InputAsset :
        public engine::reflow::Input<asset_guid> {
    public:
        using this_type = InputAsset;

    public:
        InputAsset();

        ~InputAsset() override = default;

    private:
        void setup();

    public:
        [[nodiscard]] string getTag() const noexcept override;

    public:
        engine::reflow::EventResponse onDrop(cref<engine::reflow::DragDropEvent> event_) override;

        engine::reflow::EventResponse onDragOver(cref<engine::reflow::DragDropEvent> event_) override;

    protected:
        sptr<engine::reflow::Image> _preview;
        sptr<engine::reflow::InputText> _input;
        sptr<engine::reflow::Button> _search;
        sptr<engine::reflow::Button> _reset;
        sptr<engine::reflow::Box> _content;

        engine::reflow::Children _children;

    public:
        [[nodiscard]] const ptr<const engine::reflow::Children> children() const override;

    public:
        void render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) override;

        void flow(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_,
            ref<engine::reflow::StyleKeyStack> styleStack_) override;

        void shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;

        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;

        [[nodiscard]] math::vec2 screenOffset() const noexcept override;

    public:
        void enable() override;

        void disable() override;

        void markAsDirty() override;

        void markAsPristine() override;

        void markAsTouched() override;

        void markAsUntouched() override;

        void reset() override;

        void updateValueAndValidity(const bool propagate_, const bool emit_) override;

    protected:
        asset_guid _value;

    public:
        [[nodiscard]] input_type value() const noexcept override;

        void setValue(cref<asset_guid> assetGuid_);
    };

}
