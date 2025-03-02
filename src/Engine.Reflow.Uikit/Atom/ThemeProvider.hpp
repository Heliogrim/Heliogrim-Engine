#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow::uikit {
	class ThemeProvider final :
		public Widget,
		public Atom<void, void, void>,
		public theming::ThemeProvisioner {
	public:
		using this_type = ThemeProvider;

	public:
		ThemeProvider() = default;

		~ThemeProvider() override = default;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		Vector<theming::Theme> _providerThemes;

	public:
		[[nodiscard]] std::span<const theming::Theme> getProviderThemes() const noexcept;

		void setProviderThemes(mref<Vector<theming::Theme>> themes_);

	private:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const Children> children() const override;

		void setChild(cref<SharedPtr<Widget>> nextChild_);

		Opt<ref<const ThemeProvisioner>> findParentProvisioner() const noexcept override;

		[[nodiscard]] Opt<ref<const ThemeProvisioner>> cachedUpdateNearestProvisioner(bool localInvalidate_) noexcept override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
	};
}
