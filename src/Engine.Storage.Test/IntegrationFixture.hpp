#pragma once

#include "pch.h"

/**/

#include <Engine.Config/Config.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Boot/Action/ActionResolverInit.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

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
		}

	public:
		hg::engine::storage::StorageRegistry _registry {};
		hg::engine::storage::StorageActionResolver _resolver {};
		hg::engine::storage::StorageSystem _system { _registry, _resolver };
	};
}
