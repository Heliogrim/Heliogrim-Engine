#pragma once

#include <Engine.Reflow.Attributes/Layout/CompositeLayoutAttributes.hpp>

#include "GridLayoutBase.hpp"
#include "../Atom.hpp"

namespace hg::engine::reflow::uikit {
	/**
	 * @details
	 *	Note: We will only support eXAxis as main axis within the uniform grid system right now.
	 *	Note: We require the slot-layout attributes to provide a static (not relative) sizing.
	 */
	class UniformGridLayout :
		public GridLayoutBase,
		public Atom<void, CompositeLayoutAttributes<BoxLayoutAttributes, FlexLayoutAttributes, BoxLayoutAttributes>, void> {
	public:
		using this_type = UniformGridLayout;

	public:
		UniformGridLayout();

		UniformGridLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~UniformGridLayout() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		[[nodiscard]] PrefetchSizing prefetchSlotSizingX() const;

		[[nodiscard]] PrefetchSizing prefetchSlotSizingY() const;

		[[nodiscard]] PassPrefetchSizing passPrefetchSlotSizing(ReflowAxis axis_, PrefetchSizing prefetch_, math::fvec2 reference_) const;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;
	};
}
