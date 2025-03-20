#pragma once

#include <Engine.Reflow/Widget/LeafWidget.hpp>
#include <Engine.Reflow.Attributes/Data/Base/ImageDataAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow::uikit {
	class Image :
		public LeafWidget,
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
		std::array<math::vec2, 4> _uvs;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 getShrinkFactor() const noexcept override;

		math::fvec2 getGrowFactor() const noexcept override;
	};
}
