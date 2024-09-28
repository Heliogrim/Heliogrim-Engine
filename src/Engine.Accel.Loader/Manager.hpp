#pragma once

#include <Engine.Accel.Storage/GlobalStorage.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Resource/ImporterManager.hpp>
#include <Engine.Resource/LoaderManager.hpp>

namespace hg::engine::accel {
	extern void register_loader(
		_In_ ref<resource::LoaderManager> manager_,
		_In_ mref<smr<GlobalStorage>> globalStorage_
	);

	extern void unregister_loader(
		_In_ ref<resource::LoaderManager> manager_,
		cref<smr<GlobalStorage>> globalStorage_
	);
}
