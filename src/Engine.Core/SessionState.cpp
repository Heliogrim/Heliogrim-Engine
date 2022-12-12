#include "SessionState.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.ACS/Registry.hpp>

using namespace ember::engine::core;
using namespace ember;

SessionState::SessionState() :
    _registry(make_uptr<::ember::engine::acs::Registry>()) {}

SessionState::~SessionState() = default;

const non_owning_rptr<ember::engine::acs::Registry> SessionState::getRegistry() const noexcept {
    return _registry.get();
}
