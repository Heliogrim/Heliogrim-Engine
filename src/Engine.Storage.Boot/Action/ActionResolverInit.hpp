#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage {
	class StorageSystem;
	class StorageActionResolver;
}

namespace hg::engine::storage {
	void initActionResolver(_Inout_ ref<StorageSystem> system_, _Inout_ ref<StorageActionResolver> resolver_);
}
