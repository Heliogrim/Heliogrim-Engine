#pragma once

#include <Engine.Reflow/ReflowPhase.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	// shortcut: writes through a leaf (get<>().update(), DataWatcher) bypass the owner link and keep their hand-written markAsPending; step 2 bindings replace them.
	struct AttributeOwnerLink {
	public:
		constexpr AttributeOwnerLink() noexcept = default;

		/* A copied or moved set belongs to no widget until it is bound. */
		constexpr AttributeOwnerLink(ref<const AttributeOwnerLink>) noexcept {}

		constexpr AttributeOwnerLink(mref<AttributeOwnerLink>) noexcept {}

	public:
		constexpr ref<AttributeOwnerLink> operator=(ref<const AttributeOwnerLink>) noexcept {
			return *this;
		}

		constexpr ref<AttributeOwnerLink> operator=(mref<AttributeOwnerLink>) noexcept {
			return *this;
		}

	public:
		nmpt<Widget> owner = nullptr;

	public:
		void notify(const Flag<ReflowPhase, u8> phases_) const noexcept {
			// An unbound set has no widget to mark; widgets bind their sets in the constructor (bindAttributeOwner).
			if (owner != nullptr) {
				notifyAttributeChange(owner, phases_);
			}
		}
	};
}
