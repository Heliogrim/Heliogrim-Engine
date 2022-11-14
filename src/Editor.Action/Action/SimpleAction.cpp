#include "SimpleAction.hpp"

using namespace ember::editor;
using namespace ember;

SimpleAction::SimpleAction(cref<action_type_id> typeId_) :
    Action(typeId_),
    _running(),
    _finished() {}

SimpleAction::~SimpleAction() noexcept = default;

void SimpleAction::setRunning() {
    _finished.clear();
    _running.test_and_set();
}

void SimpleAction::setFinished() {
    _running.clear();
    _finished.test_and_set();
}

bool SimpleAction::isPending() const noexcept {
    return not _running.test() && not _finished.test();
}

bool SimpleAction::isRunning() const noexcept {
    return _running.test();
}

bool SimpleAction::isFinished() const noexcept {
    return _finished.test();
}

SimpleAction::operator ptr<await_signal_sub_type>() const noexcept {
    return &_finished;
}
