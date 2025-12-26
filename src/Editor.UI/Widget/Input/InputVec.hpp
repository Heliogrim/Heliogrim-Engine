#pragma once

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

#include "./InputFloat.hpp"
#include "./InputIntegral.hpp"

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
			InputIntegral,
			InputFloat
		>;

		constexpr inline static u64 vector_dim { sizeof(vector_type) / sizeof(vector_value_base_type) };

	protected:
		InputVec() :
			engine::reflow::Input<VectorType_>(
				make_sptr<engine::reflow::uikit::HorizontalLayout>(),
				engine::reflow::ReflowClassList {},
				nullptr
			),
			_content(
				std::static_pointer_cast<engine::reflow::uikit::HorizontalLayout>(engine::reflow::CompoundWidget::_children.getChild())
			) {
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
			::hg::assertd(static_cast<bool>(_content));

			auto& attrs = _content->getLayoutAttributes().attributeSets;
			std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::maxWidth>({ engine::reflow::ReflowUnitType::eRelative, 1.F });
			std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::minHeight>({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
			std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::maxHeight>({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
			std::get<1>(attrs).update<engine::reflow::attr::FlexLayout::colGap>(4.F);
			std::get<1>(attrs).update<engine::reflow::attr::FlexLayout::rowGap>(8.F);
			std::get<1>(attrs).update<engine::reflow::attr::FlexLayout::justify>(engine::reflow::ReflowSpacing::eSpaceBetween);

			/**/

			for (u64 i = 0uLL; i < vector_dim; ++i) {
				_labels[i] = make_sptr<engine::reflow::uikit::Text>();
				_inputs[i] = make_sptr<input_widget_type>();
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
		sptr<engine::reflow::uikit::HorizontalLayout> _content;

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
		void unwindInputs(
			const ptr<const sptr<input_widget_type>> inputs_,
			ptr<vector_value_base_type> dst_
		) const {
			if constexpr (Index_ > 0) {
				dst_[Index_ - 1] = static_cast<vector_value_base_type>(inputs_[Index_ - 1]->value());
				unwindInputs<Index_ - 1>(inputs_, dst_);
			}
		}

		[[nodiscard]] VectorType_ value() const noexcept override {
			VectorType_ value {};
			unwindInputs<vector_dim>(_inputs.data(), &(value[0]));
			return value;
		}

		template <size_t Index_>
		void unwindStore(const ptr<sptr<input_widget_type>> inputs_, ptr<vector_value_base_type> src_) {
			if constexpr (Index_ > 0) {
				inputs_[Index_ - 1]->setValue(src_[Index_ - 1]);
				unwindStore<Index_ - 1>(inputs_, src_);
			}
		}

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

			if (not::hg::engine::reflow::CompoundWidget::_children.empty()) {
				::hg::engine::reflow::CompoundWidget::_children.clear();
			}

			_content->setParent(nullptr);
			_content->clearChildren();
		}

		void storeContent() {

			using namespace ::hg::engine::reflow;

			if (not _content) {
				return;
			}

			if (::hg::engine::reflow::CompoundWidget::_children.empty()) {
				::hg::engine::reflow::CompoundWidget::_children.push_back(_content);
			}

			const auto frac = 1.F / ((float)vector_dim);

			for (u64 i { 0uLL }; i < vector_dim; ++i) {

				const auto& label { _labels[i] };
				const auto& input { _inputs[i] };

				/**/

				auto wrapper = make_sptr<uikit::HorizontalLayout>();
				auto& attrs = wrapper->getLayoutAttributes().attributeSets;
				std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, frac });
				std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, frac });
				std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.F });
				std::get<0>(attrs).update<engine::reflow::attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.F });
				std::get<1>(attrs).update<engine::reflow::attr::FlexLayout::rowGap>(4.F);

				/**/

				wrapper->addChild(label);
				wrapper->addChild(input);

				_content->addChild(wrapper);
			}
		}

	protected:
		Array<sptr<engine::reflow::uikit::Text>, vector_dim> _labels;
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
		[[nodiscard]] engine::reflow::PrefetchSizing prefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowState> state_
		) const override {
			_content->setParent(const_cast<this_type*>(this)->shared_from_this());
			return {
				_content->getLayoutState().prefetchMinSize,
				_content->getLayoutState().prefetchSize
			};
		}

		engine::reflow::PassPrefetchSizing passPrefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowPassState> passState_
		) const override {
			return _content->passPrefetchSizing(axis_, passState_);
		};

		void computeSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowPassState> passState_
		) override {
			_content->getLayoutState().computeSize = passState_.computeSize;
		}

		void applyLayout(ref<engine::reflow::ReflowState> state_) override {
			const auto childState = state_.getStateOf(_content);
			childState->layoutOffset = this->getLayoutState().layoutOffset;
			childState->layoutSize = this->getLayoutState().layoutSize;
		}

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override {
			return _content->getShrinkFactor();
		}

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override {
			return _content->getGrowFactor();
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
