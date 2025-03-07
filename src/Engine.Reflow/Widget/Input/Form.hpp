#pragma once

#include <Engine.Event/EventEmitter.hpp>

#include "../Widget.hpp"
#include "../../Children.hpp"
#include "../../Event/FormSubmitEvent.hpp"

namespace hg::engine::reflow {
	class Form :
		public Widget {
	public:
		using this_type = Form;

	public:
		Form();

		~Form() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

	public:
		void setContent(cref<sptr<Widget>> content_);

		[[nodiscard]] sptr<Widget> getContent() const noexcept;

	public:
		void submitForm();

		void clearForm();

	private:
		StatelessEventEmitter<FormSubmitEvent> _submitEmitter;

	public:
		[[nodiscard]] decltype(_submitEmitter)::handle_type onSubmit(
			mref<decltype(_submitEmitter)::function_type> fnc_
		);

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;
	};
}
