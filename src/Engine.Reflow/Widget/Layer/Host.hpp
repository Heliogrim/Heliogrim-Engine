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

		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
	};
}
