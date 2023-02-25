#pragma once

#include <format>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Box.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>

#include "../../Style/Style.hpp"
#include "../../Color/Dark.hpp"

#include "Engine.GFX.Glow.UI/TestUI.hpp"

namespace ember::editor::ui {
    namespace __inputvec {
        [[nodiscard]] sptr<engine::reflow::BoundStyleSheet> makeInputBoxStyle();

        [[nodiscard]] sptr<engine::reflow::BoundStyleSheet> makeInputTextStyle();
    }

    template <typename VectorType_>
    class InputVec :
        public engine::reflow::Input<VectorType_> {
    public:
        using this_type = InputVec<VectorType_>;
        using input_type = typename engine::reflow::Input<VectorType_>::input_type;

        using vector_type = VectorType_;
        using vector_value_base_type = typename vector_type::value_type;

        using input_widget_type = _STD conditional_t<
            _STD is_integral_v<vector_value_base_type>,
            engine::reflow::InputIntegral,
            engine::reflow::InputFloat
        >;

        constexpr inline static u64 vector_dim { sizeof(vector_type) / sizeof(vector_value_base_type) };

    protected:
        InputVec() :
            engine::reflow::Input<VectorType_>() {
            prepare();
        }

    public:
        ~InputVec() override = default;

    public:
        [[nodiscard]] string getTag() const noexcept override {
            return _STD format(R"(InputVec[{}] <{:#x}>)", vector_dim, reinterpret_cast<u64>(this));
        }

    public:
        void prepare() {

            auto localStyle = engine::reflow::StyleSheet {
                .width = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eRelative, 1.F }
                },
                .maxWidth = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eRelative, 1.F }
                },
                .minHeight = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eAbsolute, 20.F }
                },
                .height = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eAbsolute, 20.F }
                },
                .maxHeight = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eAbsolute, 20.F }
                },
                .wrap = { true, engine::reflow::ReflowWrap::eNoWrap },
                .colGap = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eAbsolute, 4.F }
                },
                .rowGap = {
                    true, engine::reflow::ReflowUnit { engine::reflow::ReflowUnitType::eAbsolute, 8.F }
                },
                .reflowSpacing = { true, engine::reflow::ReflowSpacing::eSpaceBetween },
                .reflowShrink = { true, 1.F },
                .reflowGrow = { true, 0.F },
                .color = { true, color::Dark::backgroundDefault }
            };

            _content = make_sptr<engine::reflow::HBox>(engine::reflow::BoundStyleSheet::make(_STD move(localStyle)));

            /**/

            for (u64 i = 0ui64; i < vector_dim; ++i) {

                _labels[i] = make_sptr<engine::reflow::Text>(
                    engine::reflow::BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))
                );
                _labels[i]->style().minHeight = engine::reflow::ReflowUnit {
                    engine::reflow::ReflowUnitType::eRelative, 1.F
                };
                _labels[i]->style().height = engine::reflow::ReflowUnit {
                    engine::reflow::ReflowUnitType::eRelative, 1.F
                };
                _labels[i]->style().maxHeight = engine::reflow::ReflowUnit {
                    engine::reflow::ReflowUnitType::eRelative, 1.F
                };
                _labels[i]->style().color = color::Dark::white;
                _labels[i]->style().textEllipse = 1ui32;

                _inputs[i] = make_sptr<input_widget_type>(
                    __inputvec::makeInputBoxStyle(),
                    __inputvec::makeInputTextStyle()
                );
            }

            /**/

            for (u64 i { 0ui64 }; i < vector_dim; ++i) {
                _inputs[i]->_callback = [this](auto) {
                    this->onInternalChange();
                };
            }

            /**/

            storeContent();
        }

    protected:
        sptr<engine::reflow::Box> _content;
        engine::reflow::Children _children;

    public:
        [[nodiscard]] const ptr<const engine::reflow::Children> children() const override {
            return &_children;
        }

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override {
            return _content->outerSize();
        }

        [[nodiscard]] math::vec2 innerSize() const noexcept override {
            return _content->innerSize();
        }

        [[nodiscard]] math::vec2 screenOffset() const noexcept override {
            return _content->screenOffset();
        }

    public:
        void enable() override {
            for (const auto& input : _inputs) {
                input->enable();
            }
        }

        void disable() override {
            for (const auto& input : _inputs) {
                input->disable();
            }
        }

        void markAsDirty() override {
            for (const auto& input : _inputs) {
                input->markAsDirty();
            }
        }

        void markAsPristine() override {
            for (const auto& input : _inputs) {
                input->markAsPristine();
            }
        }

        void markAsTouched() override {
            for (const auto& input : _inputs) {
                input->markAsTouched();
            }
        }

        void markAsUntouched() override {
            for (const auto& input : _inputs) {
                input->markAsUntouched();
            }
        }

        void reset() override {
            for (const auto& input : _inputs) {
                input->reset();
            }
        }

        void updateValueAndValidity(const bool propagate_, const bool emit_) override {
            for (const auto& input : _inputs) {
                input->updateValueAndValidity(true, emit_);
            }
        }

    public:
        template <size_t Index_>
        FORCE_INLINE void unwindInputs(
            const ptr<const sptr<input_widget_type>> inputs_,
            ptr<vector_value_base_type> dst_
        ) const {
            dst_[Index_ - 1] = inputs_[Index_ - 1]->value();
            unwindInputs<Index_ - 1>(inputs_, dst_);
        }

        template <>
        FORCE_INLINE void unwindInputs<0>(
            const ptr<const sptr<input_widget_type>> inputs_,
            ptr<vector_value_base_type> dst_
        ) const {}

        [[nodiscard]] VectorType_ value() const noexcept override {
            VectorType_ value {};
            unwindInputs<vector_dim>(_inputs.data(), &(value[0]));
            return value;
        }

        template <size_t Index_>
        FORCE_INLINE void unwindStore(const ptr<sptr<input_widget_type>> inputs_, ptr<vector_value_base_type> src_) {
            inputs_[Index_ - 1]->setValue(src_[Index_ - 1]);
            unwindStore<Index_ - 1>(inputs_, src_);
        }

        template <>
        FORCE_INLINE void unwindStore<0>(
            const ptr<sptr<input_widget_type>> inputs_,
            ptr<vector_value_base_type> src_
        ) {}

        void setValue(cref<VectorType_> value_) {
            auto tmp { value_ };
            unwindStore<vector_dim>(_inputs.data(), &(tmp[0]));
        }

    private:
    public:
        _STD function<void(input_type)> _callback = nullptr;

        void onInternalChange() {

            if (not _callback) {
                return;
            }

            const auto val { value() };
            _callback(val);
        }

    protected:
        void clearContent() {

            if (not _content) {
                return;
            }

            if (not _children.empty()) {
                _children.clear();
            }

            _content->setParent(nullptr);
            _content->clearChildren();
        }

        void storeContent() {

            using namespace ::ember::engine::reflow;

            if (not _content) {
                return;
            }

            if (_children.empty()) {
                _children.push_back(_content);
            }

            const auto frac = 1.F / ((float)vector_dim);

            for (u64 i { 0ui64 }; i < vector_dim; ++i) {

                const auto& label { _labels[i] };
                const auto& input { _inputs[i] };

                /**/

                auto wrapper {
                    make_sptr<HBox>(
                        BoundStyleSheet::make(
                            StyleSheet {
                                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, frac } },
                                .width { true, ReflowUnit { ReflowUnitType::eRelative, frac } },
                                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, frac } },
                                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                                .wrap { true, ReflowWrap::eNoWrap },
                                .rowGap { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
                                .reflowShrink { true, 1.F },
                                .reflowGrow { true, 0.F },
                                .color { false, color::Dark::backgroundDefault }
                            }
                        )
                    )
                };

                /**/

                wrapper->addChild(label);
                wrapper->addChild(input);

                _content->addChild(wrapper);
            }
        }

    protected:
        Array<sptr<engine::reflow::Text>, vector_dim> _labels;
        Array<sptr<input_widget_type>, vector_dim> _inputs;

    public:
        [[nodiscard]] cref<string> getLabel(const u64 idx_) const noexcept {
            return _labels[idx_]->getText();
        }

        void setLabel(const u64 idx_, cref<string> label_) {
            _labels[idx_]->setText(label_);
        }

    public:
        void render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) override {
            if (_content) {
                _content->render(cmd_);
            }
        }

        void flow(
            cref<engine::reflow::FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<engine::reflow::StyleKeyStack> styleStack_
        ) override {
            if (_content) {
                _content->setParent(engine::reflow::Widget::shared_from_this());
                _content->flow(ctx_, space_, limit_, styleStack_);
            }

            engine::reflow::Widget::clearPending();
        }

        void shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) override {
            if (_content) {
                _content->shift(ctx_, offset_);
            }
        }
    };

    class InputVec3 :
        public InputVec<math::vec3> {
    public:
        using this_type = InputVec3;

    public:
        InputVec3() = default;

        ~InputVec3() override = default;
    };
}
