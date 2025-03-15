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

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;
	};
}
