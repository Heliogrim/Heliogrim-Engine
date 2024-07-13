#pragma once
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class Config;
	class StorageModule;
}

namespace hg::editor::boot {
	void initRuntimeStorage(_In_ cref<engine::Config> cfg_, _In_ ref<engine::StorageModule> module_);
}
