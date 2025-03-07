#pragma once

#include "../Widget.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
	class Host final :
		public Widget {
	public:
		using this_type = Host;

	public:
		explicit Host(_In_ ref<const sptr<Widget>> layout_);

		Host(_In_ ref<const sptr<Widget>> layout_, _In_ math::vec2 hostOffset_, _In_ math::vec2 hostSize_);

		~Host() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		SingleChildren _layout;
		math::vec2 _hostOffset;
		math::vec2 _hostSize;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

		[[nodiscard]] sptr<Widget> layout() const noexcept;

		[[nodiscard]] math::vec2 getHostOffset() const noexcept;

		[[nodiscard]] math::vec2 getHostSize() const noexcept;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;
	};
}
