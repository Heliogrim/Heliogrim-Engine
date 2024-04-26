#pragma once

#include <format>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/BoxPanel.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

#include "../../Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"

#include "Engine.GFX.Glow.UI/TestUI.hpp"

namespace hg::editor::ui {
	template <typename VectorType_>
	class InputVec :
		public engine::reflow::Input<VectorType_> {
	public:
		using this_type = InputVec<VectorType_>;
		using input_type = typename engine::reflow::Input<VectorType_>::input_type;

		using vector_type = VectorType_;
		using vector_value_base_type = typename vector_type::value_type;

		using input_widget_type = std::conditional_t<
			std::is_integral_v<vector_value_base_type>,
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
			return std::format(R"(InputVec[{}] <{:#x}>)", vector_dim, reinterpret_cast<u64>(this));
		}

	public:
		void prepare() {

			const auto theme = Theme::get();

			_content = make_sptr<engine::reflow::HorizontalPanel>();
			_content->attr.width.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
			_content->attr.maxWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
			_content->attr.minHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
			_content->attr.height.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
			_content->attr.maxHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
			_content->attr.colGap.setValue(4.F);
			_content->attr.rowGap.setValue(8.F);
			_content->attr.justify.setValue(engine::reflow::ReflowSpacing::eSpaceBetween);
			_content->attr.flexShrink.setValue(1.F);

			/**/

			for (u64 i = 0uLL; i < vector_dim; ++i) {

				_labels[i] = make_sptr<engine::reflow::Text>();
				_labels[i]->attr.textEllipse.setValue(1uL);
				theme->applyLabel(_labels[i]);

				_inputs[i] = make_sptr<input_widget_type>(); {
					auto& style = _inputs[i]->_wrapper->attr;

					style.minWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
					style.width.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
					style.maxWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });

					style.minHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
					style.height.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
					style.maxHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });

					style.padding.setValue(engine::reflow::Padding { 4.F, 2.F });
					style.flexShrink.setValue(1.F);

					style.style.setValue(
						engine::reflow::PanelStyle { .backgroundColor = engine::color { 40.F, 44.F, 52.F, 255.F } }
					);
				}

				theme->applyText(_inputs[i]->_text);
			}

			/**/

			for (u64 i { 0uLL }; i < vector_dim; ++i) {
				_inputs[i]->_callback = [this](auto) {
					this->onInternalChange();
				};
			}

			/**/

			storeContent();
		}

	protected:
		sptr<engine::reflow::HorizontalPanel> _content;
		engine::reflow::Children _children;

	public:
		[[nodiscard]] const ptr<const engine::reflow::Children> children() const override {
			return &_children;
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
			dst_[Index_ - 1] = static_cast<vector_value_base_type>(inputs_[Index_ - 1]->value());
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
		std::function<void(input_type)> _callback = nullptr;

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

			using namespace ::hg::engine::reflow;

			if (not _content) {
				return;
			}

			if (_children.empty()) {
				_children.push_back(_content);
			}

			const auto frac = 1.F / ((float)vector_dim);

			for (u64 i { 0uLL }; i < vector_dim; ++i) {

				const auto& label { _labels[i] };
				const auto& input { _inputs[i] };

				/**/

				auto wrapper = make_sptr<HorizontalPanel>();
				wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, frac });
				wrapper->attr.width.setValue({ ReflowUnitType::eRelative, frac });
				wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, frac });
				wrapper->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
				wrapper->attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
				wrapper->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
				wrapper->attr.rowGap.setValue(4.F);
				wrapper->attr.flexShrink.setValue(1.F);
				wrapper->attr.flexGrow.setValue(0.F);

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
		void render(
			const ptr<engine::reflow::ReflowCommandBuffer> cmd_
		) override {
			_content->render(cmd_);
		}

	public:
		[[nodiscard]] math::vec2 prefetchDesiredSize(
			cref<engine::reflow::ReflowState> state_,
			const float scale_
		) const override {
			_content->setParent(const_cast<this_type*>(this)->shared_from_this());
			return _content->prefetchDesiredSize(state_, scale_);
		}

		[[nodiscard]] math::vec2 computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const override {
			return _content->computeDesiredSize(passState_);
		}

		void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override {
			const auto state = state_.getStateOf(_content);
			state->layoutOffset = ctx_.localOffset;
			state->layoutSize = ctx_.localSize;
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

	class InputUiVec2 :
		public InputVec<math::uivec2> {
	public:
		using this_type = InputUiVec2;

	public:
		InputUiVec2() = default;

		~InputUiVec2() override = default;
	};

	class InputUiVec3 :
		public InputVec<math::uivec3> {
	public:
		using this_type = InputUiVec3;

	public:
		InputUiVec3() = default;

		~InputUiVec3() override = default;
	};
}
