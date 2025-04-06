#include "DelegateAction.hpp"

using namespace hg::editor;
using namespace hg;

DelegateAction::DelegateAction(mref<delegate_type> fwd_, mref<delegate_type> bwd_) :
	InheritMeta(),
	_fwd(std::move(fwd_)),
	_bwd(std::move(bwd_)) {}

DelegateAction::~DelegateAction() = default;

void DelegateAction::store(mref<delegate_type> fwd_, mref<delegate_type> bwd_) {
	_fwd = std::move(fwd_);
	_bwd = std::move(bwd_);
}

bool DelegateAction::isReversible() const noexcept {
	return _bwd != nullptr;
}

Result<void, std::runtime_error> DelegateAction::apply() {
	setRunning();
	_fwd();
	setFinished();
	return Expected<void> {};
}

Result<void, std::runtime_error> DelegateAction::revoke() {
	return Unexpected { std::runtime_error { "Unable to revoke delegate actions." } };
}

Result<void, std::runtime_error> DelegateAction::undo() {
	setRunning();
	_bwd();
	setFinished();
	return Expected<void> {};
}
