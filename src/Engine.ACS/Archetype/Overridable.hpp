#pragma once

namespace hg::engine::acs {
	/**
	 * Marks a layout as overridable: `Extend<Overridable<L>, Override<...>>` may change base slot offsets.
	 *
	 * The archetype owning such a layout writes its logic as deducing-this bodies
	 * (`void tick(this auto& self)` in its `.inl.hpp`), so each derived layout resolves slots itself.
	 * Without the marker, an `Override` that moves a base slot fails to compile.
	 *
	 * The marker belongs to one layout level; a layout extending an overridable base is not overridable
	 * unless it is wrapped again.
	 */
	template <typename Layout_>
	struct Overridable {
		using layout = Layout_;
	};

	template <typename>
	inline constexpr bool overridable_v = false;

	template <typename Layout_>
	inline constexpr bool overridable_v<Overridable<Layout_>> = true;
}
