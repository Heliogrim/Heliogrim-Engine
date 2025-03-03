#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

#include "../Molecule/Layout/Stack.hpp"

namespace hg::engine::reflow::uikit {
	class Card :
		public uikit::Stack,
		public theming::ThemeProvisioner {
	public:
		using this_type = Card;

	public:
		Card();

		~Card() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		Opt<theming::Theme> _theme;

	public:
		[[nodiscard]] ref<const Opt<theming::Theme>> getCardTheme() const noexcept;

		void setCardTheme(mref<Opt<theming::Theme>> theme_);

	public:
		[[nodiscard]] Opt<ref<const ThemeProvisioner>> findParentProvisioner() const noexcept override;

		Opt<ref<const theming::ThemeProvisioner>> cachedUpdateNearestProvisioner(bool localInvalidate_) noexcept override;
	};

	/**/

	struct CardCreateOptions {
		u8 level;
		SharedPtr<Widget>&& children;
	};

	[[nodiscard]] extern SharedPtr<Card> makeCard(mref<CardCreateOptions> options_);
}
