#include "Session.hpp"

#include <Heliogrim/Async/Future.hpp>
#include <Engine.Core/Session.hpp>

#include "../Heliogrim.hpp"

using namespace hg;

Session::Session(mref<SharedPtr<::hg::engine::core::Session>> internal_) :
	_internal(std::move(internal_)) {}

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
	return Future<bool>::ready(false);
}
