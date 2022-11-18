#include "DelegateAction.hpp"

using namespace ember::editor;
using namespace ember;

DelegateAction::DelegateAction(cref<action_type_id> typeId_) :
    SimpleAction(typeId_),
    _fwd(nullptr),
    _bwd(nullptr) {}

DelegateAction::DelegateAction() :
    SimpleAction(DelegateAction::typeId),
    _fwd(nullptr),
    _bwd(nullptr) {}

DelegateAction::DelegateAction(mref<delegate_type> fwd_, mref<delegate_type> bwd_) :
    SimpleAction(DelegateAction::typeId),
    _fwd(_STD move(fwd_)),
    _bwd(_STD move(bwd_)) {}

DelegateAction::~DelegateAction() = default;

void DelegateAction::store(mref<delegate_type> fwd_, mref<delegate_type> bwd_) {
    _fwd = _STD move(fwd_);
    _bwd = _STD move(bwd_);
}

bool DelegateAction::isReversible() const noexcept {
    return _bwd != nullptr;
}

void DelegateAction::apply() {
    setRunning();
    _fwd();
    setFinished();
}

void DelegateAction::reverse() {
    setRunning();
    _bwd();
    setFinished();
}

bool DelegateAction::failed() const noexcept {
    return false;
}
