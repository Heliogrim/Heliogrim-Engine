#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	class StorageSystem;
	class StorageActionResolver;
}

namespace hg::engine::storage {
	void initActionResolver(_Inout_ ref<StorageSystem> system_, _Inout_ ref<StorageActionResolver> resolver_);
}
