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
		void show(_Inout_ sptr<Widget> content_, _In_ math::vec2 globalOffset_);

		void hide();

	public:
		[[nodiscard]] const ptr<const NullChildren> children() const override;

		void render(ptr<ReflowCommandBuffer> cmd_) override;

	public:
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

		ReflowPosition position() const noexcept override;
	};
}
