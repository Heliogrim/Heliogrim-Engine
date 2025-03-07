#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Data/Base/ImageDataAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow::uikit {
	class Image :
		public Widget,
		public Atom<
			ImageDataAttributes,
			BoxLayoutAttributes,
			ImageStyleAttributes
		> {
	public:
		using this_type = Image;

	public:
		Image();

		Image(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Image() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		NullChildren _children;
		std::array<math::vec2, 4> _uvs;

	public:
		const ptr<const NullChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getShrinkFactor() const noexcept override;

		math::fvec2 getGrowFactor() const noexcept override;
	};
}
