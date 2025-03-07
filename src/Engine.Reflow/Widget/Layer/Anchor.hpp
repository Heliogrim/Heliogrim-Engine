#pragma once

#include "Host.hpp"
#include "../Widget.hpp"

namespace hg::engine::reflow {
	class Anchor final :
		public Widget {
	public:
		using this_type = Anchor;

	public:
		Anchor();

		~Anchor() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		sptr<Host> _host;

	public:
		[[nodiscard]] bool isShown() const noexcept;

		void show(_Inout_ sptr<Widget> content_, _In_ math::vec2 globalOffset_);

		void hide();

	public:
		[[nodiscard]] const ptr<const NullChildren> children() const override;

		void render(ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;

		ReflowPosition position() const noexcept override;
	};
}
