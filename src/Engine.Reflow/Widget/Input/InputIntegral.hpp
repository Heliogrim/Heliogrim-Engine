#pragma once

#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

#include "../Input.hpp"

namespace hg::engine::reflow {
	class InputIntegral :
		public Input<s64> {
	public:
		using this_type = InputIntegral;

	public:
		InputIntegral();

		~InputIntegral() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
	public:
		sptr<uikit::HorizontalLayout> _wrapper;
		sptr<uikit::Text> _text;

	private:
		string _value;

		math::vec2_t<s64> _limits;
		s64 _stepSize;

	public:
		void setValue(const s64 value_);

		void setLimits(
			const s64 lower_ = std::numeric_limits<s64>::min(),
			const s64 upper_ = std::numeric_limits<s64>::max()
		);

		void setStepSize(const s64 stepSize_);

	public:
		void updateValueAndValidity(const bool propagate_, const bool emit_) override;

	public:
		std::function<void(input_type)> _callback = nullptr;

	public:
		[[nodiscard]] input_type value() const noexcept override;

	private:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;

	public:
		EventResponse invokeOnFocus(cref<FocusEvent> event_) override;

		EventResponse invokeOnBlur(cref<FocusEvent> event_) override;

		EventResponse invokeOnKeyDown(ref<const KeyboardEvent> event_) override;

		EventResponse invokeOnKeyUp(ref<const KeyboardEvent> event_) override;
	};
}
