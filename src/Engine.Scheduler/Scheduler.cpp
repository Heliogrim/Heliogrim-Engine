#include "Scheduler.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

constinit std::atomic<ptr<engine::Scheduler>> engine::Scheduler::_moduleSlot { nullptr };
static_assert(engine::core::ModuleSlotAccess::published<engine::Scheduler>);
