#include "RootScopedSlot.hpp"

#include <Engine.Common/__macro.hpp>

#include "RootSlot.hpp"

import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

RootScopedSlot::RootScopedSlot(_Inout_ ref<resource::Archive> archive_, const ScopedSlotStateFlags flags_, nmpt<LoadContext> context_) :
	RecordScopedSlot(ScopedSlotState { flags_ }, RootSlot::makeRootState(archive_, context_)) {}
