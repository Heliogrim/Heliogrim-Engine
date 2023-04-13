#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow/Widget/BoxPanel.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
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
        sptr<engine::reflow::VerticalPanel> _content;

        engine::reflow::Children _children;

    public:
        [[nodiscard]] const ptr<const engine::reflow::Children> children() const override;

    public:
        void render(
            cref<engine::reflow::ReflowState> state_,
            const ptr<engine::reflow::ReflowCommandBuffer> cmd_
        ) override;

    public:
        math::vec2 prefetchDesiredSize(cref<engine::reflow::ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const override;

        void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override;

    public:
        [[nodiscard]] float shrinkFactor() const noexcept override;

        [[nodiscard]] float growFactor() const noexcept override;

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

        /**/

    protected:
        Vector<asset_type_id> _acceptedTypes;

    public:
        [[nodiscard]] cref<Vector<asset_type_id>> acceptedTypes() const noexcept;

        void addAcceptedType(asset_type_id typeId_);

        void dropAcceptedType(asset_type_id typeId_);

    private:
    public:
        _STD function<void(input_type)> _callback = nullptr;
    };
}
