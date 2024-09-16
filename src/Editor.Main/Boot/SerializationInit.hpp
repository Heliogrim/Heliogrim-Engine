#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class Config;
	class SerializationModule;
}

namespace hg::editor::boot {
	void initSerialization(_In_ cref<engine::Config> cfg_, _In_ ref<engine::SerializationModule> module_);
}
