#pragma once

#include "../Atom/Image.hpp"
#include "../Atom/Text.hpp"
#include "../Molecule/Layout/Stack.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	class Button :
		public uikit::Stack,
		public theming::ThemeProvisioner {
	public:
		using this_type = Button;

	public:
		Button();

		~Button() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		struct MapStateToSets {
			Opt<theming::Theme> base;
			Opt<theming::Theme> focus;
			Opt<theming::Theme> hover;
			Opt<theming::Theme> pressed;
			// TODO: Check form states
			Opt<theming::Theme> selected;
			Opt<theming::Theme> disabled;
		};

		void setBaseTheme(mref<Opt<theming::Theme>> theme_);

		void setStateTheme(InteractiveStateFlagBits interactiveBit_, mref<Opt<theming::Theme>> theme_);

	public:
		[[nodiscard]] Opt<ref<const ThemeProvisioner>> findParentProvisioner() const noexcept override;

		[[nodiscard]] Opt<ref<const ThemeProvisioner>> cachedUpdateNearestProvisioner(
			bool localInvalidate_
		) noexcept override;

	protected:
		MapStateToSets _themeStateMap;

		void notifyContextChange();

	public:
		EventReply invokeOnFocus(cref<FocusEvent> event_) override;

		EventReply invokeOnBlur(cref<FocusEvent> event_) override;

		EventReply invokeOnMouseButtonDown(ref<const MouseEvent> event_) override;

		EventReply invokeOnMouseButtonUp(ref<const MouseEvent> event_) override;

		EventReply invokeOnMouseEnter(ref<const MouseMoveEvent> event_) override;

		EventReply invokeOnMouseLeave(ref<const MouseMoveEvent> event_) override;
	};

	/**/

	struct TextButtonCreateOptions {
		u8 level;
		SharedPtr<Text>&& text;
	};

	struct ImageButtonCreateOptions {
		u8 level;
		SharedPtr<Image>&& image;
	};

	struct ButtonCreateOptions {
		u8 level;
		SharedPtr<Widget>&& children;
	};

	[[nodiscard]] extern SharedPtr<Button> makeButton(
		mref<Variant<TextButtonCreateOptions, ImageButtonCreateOptions, ButtonCreateOptions>> options_
	);
}
