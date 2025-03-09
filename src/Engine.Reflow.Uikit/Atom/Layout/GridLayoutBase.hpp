#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

namespace hg::engine::reflow::uikit {
	class GridLayoutBase :
		public Widget {
	public:
		using this_type = GridLayoutBase;

	public:
		GridLayoutBase(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~GridLayoutBase() override;

	protected:
		Children _children;

	public:
		const ptr<const Children> children() const override;

		void addChild(cref<sptr<Widget>> child_);

		void setChild(const u32 idx_, cref<sptr<Widget>> child_);

		void removeChild(cref<sptr<Widget>> child_);

		void clearChildren();

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;
	};
}
