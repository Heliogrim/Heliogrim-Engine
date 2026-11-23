#pragma once

#include "pch.h"

/**/

#include "Scratch.hpp"

/**/

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Config;
import Engine.Storage.Action;
import Engine.Storage.Boot;
import Engine.Storage.Registry;
import Engine.Storage.System;
#endif

namespace StorageModule {
	class MutationIntegrationFixture :
		public testing::Test {
	public:
		MutationIntegrationFixture() {
			hg::engine::Config cfg {};
			_registry.setup(cfg);
			initActionResolver(_system, _resolver);
		}

		~MutationIntegrationFixture() override {
			// Warning: Order-Dependent destruction
			// Note: 1. System -> Context -> Cache
			// Note: 2. System -> Context -> Store
			// Note: 3. System
			// Note: 4. Resolver
			// Note: 5. Registry
			// Note: 6. Scratch directory
		}

	public:
		// Note: Declared first so it is removed last, after the system has released every cached file inside it.
		ScratchDir _scratch {};
		hg::engine::storage::StorageRegistry _registry {};
		hg::engine::storage::StorageActionResolver _resolver {};
		hg::engine::storage::StorageSystem _system { _registry, _resolver };
	};
}
