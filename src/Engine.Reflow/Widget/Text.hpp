#pragma once
#include <Engine.Assets/Types/Font.hpp>
#include <Heliogrim/TypedAssetGuid.hpp>

#include "Widget.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"
#include "../ReflowWrap.hpp"
#include "../TextAlign.hpp"

namespace hg::engine::reflow {
	class Font;
}

namespace hg::engine::reflow {
	class Text :
		public Widget {
	public:
		using this_type = Text;

	public:
		Text();

		~Text() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		struct Attributes {
			Attribute<ReflowUnit> width;
			Attribute<ReflowUnit> height;

			Attribute<TypedAssetGuid<engine::assets::Font>> font;
			Attribute<float> fontSize;
			Attribute<float> lineHeight;

			Attribute<TextAlign> textAlign;
			Attribute<u32> textEllipse;
			Attribute<ReflowWrap> textWrap;

			Attribute<engine::color> color;

			/**/

			Attribute<string> text;
		} attr;

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
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

	public:
		[[nodiscard]] bool willChangeLayout(
			cref<math::vec2> space_
		) const noexcept override;
	};
}
