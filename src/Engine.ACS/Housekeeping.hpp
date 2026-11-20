#pragma once

#include <Heliogrim/Actor/ActorGuid.hpp>

#include "Component.hpp"
#include "ComponentTypeId.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::acs {
	class Registry;

	class Housekeeping final {
	public:
		friend class Registry;

	public:
		using this_type = Housekeeping;

	public:
		explicit Housekeeping(_Inout_ ref<Registry> registry_);

		Housekeeping(mref<this_type>) = delete;

		Housekeeping(ref<const this_type>) = delete;

		~Housekeeping();

	private:
		ref<Registry> _registry;
		/* releases arrive concurrently from parallel tick stages (actor destruction) */
		concurrent::SpinMutex _lck;
		DenseMap<ComponentTypeId, Vector<ptr<const LogicComponent>>> _pendingReleases;

	protected:
		void storePendingRelease(mref<VolatileComponent<>> component_);

	public:
		/**
		 * @returns true if any retired slot became claimable
		 */
		bool commit();
	};
}
