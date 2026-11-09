#include "SimpleAction.hpp"

using namespace hg::editor;
using namespace hg;

SimpleAction::~SimpleAction() noexcept = default;

void SimpleAction::setRunning() {
	_running.test_and_set();
}

void SimpleAction::setFinished() {
	_running.clear();
	_finished.set();
}

bool SimpleAction::isPending() const noexcept {
	return not _running.test() && not _finished.isSet();
}

bool SimpleAction::isRunning() const noexcept {
	return _running.test();
}

bool SimpleAction::isFinished() const noexcept {
	return _finished.isSet();
}

ref<Signal> SimpleAction::finished() const noexcept {
	return _finished;
}
