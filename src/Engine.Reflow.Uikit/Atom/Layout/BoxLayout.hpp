#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>

#include "../Atom.hpp"

namespace hg::engine::reflow::uikit {
	class BoxLayout :
		public Widget,
		public Atom<void, BoxLayoutAttributes, void> {
	public:
		using this_type = BoxLayout;

	public:
		BoxLayout();

		BoxLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~BoxLayout() override;

	public:
		string getTag() const noexcept override;

	protected:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const Children> children() const override;

		void setChild(cref<sptr<Widget>> nextChild_);

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;
	};
}
