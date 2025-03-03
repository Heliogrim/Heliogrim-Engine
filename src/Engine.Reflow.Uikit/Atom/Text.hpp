#pragma once

#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Data/Base/TextDataAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/TextStyleAttributes.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow {
	class Font;
}

namespace hg::engine::reflow::uikit {
	class Text :
		public Widget,
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
		math::vec2 contentSize(cref<reflow::Font> font_, cref<math::vec2> space_) const;

	private:
		NullChildren _children;

	public:
		const ptr<const NullChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
	};
}
