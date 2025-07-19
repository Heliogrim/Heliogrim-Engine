#pragma once

#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/LeafWidget.hpp>
#include <Engine.Reflow.Attributes/Data/Base/TextDataAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/TextStyleAttributes.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow {
	class Font;
}

namespace hg::engine::reflow::uikit {
	class Text :
		public LeafWidget,
		public Atom<TextDataAttributes, BoxLayoutAttributes, TextStyleAttributes> {
	public:
		using this_type = Text;

	public:
		Text();

		Text(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Text() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		[[nodiscard]] string getText() const noexcept;

		void setText(cref<string> text_);

	private:
		mutable struct TextMeasureCache {
			std::uintptr_t textRef;
			size_t textHash;
			std::uintptr_t fontRef;
			f32 fontSizeRef;
			f32 lineSizeRef;
			f32 chunkLimitRef;

			f32 minExtend;
			f32 freeExtend;
			f32 chunkSum;
		} _measureCache;

		[[nodiscard]] bool isBreakPoint(char ch_) const noexcept;

		[[nodiscard]] math::fvec2 measure1DimText(ref<const reflow::Font> font_) const;

		[[nodiscard]] f32 measure1CrossDimChunked(ref<const reflow::Font> font_, f32 chunkLimit_) const;

	public:
		void onAttachDataWatcher(ref<DataWatcher> watcher_) override;

		void onDetachDataWatcher(ref<DataWatcher> watcher_) override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;
	};
}
