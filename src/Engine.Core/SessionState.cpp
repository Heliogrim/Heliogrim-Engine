#include "SessionState.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.ACS/Registry.hpp>

using namespace hg::engine::core;
using namespace hg;

SessionState::SessionState() :
	_registry(make_uptr<::hg::engine::acs::Registry>()) {}

SessionState::~SessionState() = default;

ref<hg::engine::acs::Registry> SessionState::getRegistry() const noexcept {
	return *_registry;
}
