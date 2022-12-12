#include "Session.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>

#include "Ember.hpp"
#include "Inbuilt.hpp"

using namespace ember;

Session::Session(const managed<void> internal_) :
    _internal(internal_) {}

ActorInitializer Session::getActorInitializer() const noexcept {
    return ActorInitializer {
        _internal
    };
}

bool Session::valid() const noexcept {
    return _internal.get() && _internal.use_count();
}

cref<managed<void>> Session::unwrap() const noexcept {
    return _internal;
}

Session ember::GetSession() {
    return Ember::getSession();
}

Session ember::CreateSession() {
    return Session {};
}

Future<bool> ember::Destroy(mref<Session> session_) {
    concurrent::promise<bool> p {
        [] {
            return false;
        }
    };

    auto f { p.get() };
    p();

    return f;
}
