#include "Session.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Session.hpp>

#include "../Heliogrim.hpp"

using namespace hg;

Session::Session(mref<SharedPtr<::hg::engine::core::Session>> internal_) :
	_internal(std::move(internal_)) {}

ActorInitializer Session::getActorInitializer() const noexcept {
	return ActorInitializer {
		_internal
	};
}

bool Session::valid() const noexcept {
	return _internal.get() && _internal.use_count();
}

cref<decltype(Session::_internal)> Session::unwrap() const noexcept {
	return _internal;
}

Session hg::GetSession() {
	return Heliogrim::getSession();
}

Session hg::CreateSession() {
	return Session {};
}

Future<bool> hg::Destroy(mref<Session> session_) {
	concurrent::promise<bool> p {
		[] {
			return false;
		}
	};

	auto f { p.get() };
	p();

	return f;
}
