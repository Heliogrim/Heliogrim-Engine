#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class ActorModule;
}

namespace hg::editor::boot {
	extern void initActorClasses(_In_ ref<engine::ActorModule> module_);
}
