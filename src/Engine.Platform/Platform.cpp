#include "Platform.hpp"

using namespace hg::engine;
using namespace hg;

constinit std::atomic<ptr<engine::Platform>> engine::Platform::_moduleSlot { nullptr };
static_assert(engine::core::ModuleSlotAccess::published<engine::Platform>);

std::atomic<bool>& platform::dragActive() noexcept {
	static std::atomic<bool> active { false };
	return active;
}

Platform::Platform() noexcept = default;

Platform::~Platform() = default;
