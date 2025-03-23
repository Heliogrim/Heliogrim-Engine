#include "InputToggle.hpp"

#include "EditorUiEventNames.hpp"
#include "Engine.Common/Discard.hpp"
#include "Engine.Reflow/DataWatcher.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

InputToggle::InputToggle() :
	Input(NullWidget::instance(), ReflowClassList {}, nullptr),
	Atom({ InputToggleDataAttributes { false } }, {}, {}),
	_activeToggleListener(),
	_activeContent(),
	_inactiveToggleListener(),
	_inactiveContent() {}

InputToggle::~InputToggle() = default;

string InputToggle::getTag() const noexcept {
	return std::format(R"(InputToggle <{:#x}>)", reinterpret_cast<u64>(this));
}

bool InputToggle::shouldTick() const noexcept {
	return not state().isFocus() && isPristine() && state().isPending();
}
void InputToggle::tick() {

	if (isTouched() && isPristine()) {
		markAsUntouched();
	}

	const auto& dataValue = getDataAttributes().valueOf<attr::InputData::value>();
	setValue(dataValue);
}

void InputToggle::setActiveContent(mref<SharedPtr<Widget>> active_) {
	if (_activeContent != nullptr && _activeContent != NullWidget::instance()) {
		_activeContent->dropOnClick(_activeToggleListener);
	}

	_activeContent = hg::move(active_);

	if (InputToggle::value() == true) {
		_activeContent->setParent(shared_from_this());
		auto prev = _children.setChild(clone(_activeContent));
		prev->setParent(nullptr);
	}

	if (_activeContent != nullptr && _activeContent != NullWidget::instance()) {
		_activeToggleListener = _activeContent->onClick(
			[this]([[maybe_unused]] ref<const engine::input::event::MouseButtonEvent> event_) {
				// Warning: Maybe cause use-after-free due to immediate tree-rewrite in `setValue`
				setValue(false);
				updateValueAndValidity(true, true);
				return EventResponse::eConsumed;
			}
		);
	}
}

void InputToggle::setInactiveContent(mref<SharedPtr<Widget>> inactive_) {
	if (_inactiveContent != nullptr && _inactiveContent != NullWidget::instance()) {
		_inactiveContent->dropOnClick(_activeToggleListener);
	}

	_inactiveContent = hg::move(inactive_);

	if (InputToggle::value() == false) {
		_inactiveContent->setParent(shared_from_this());
		auto prev = _children.setChild(clone(_inactiveContent));
		prev->setParent(nullptr);
	}

	if (_inactiveContent != nullptr && _inactiveContent != NullWidget::instance()) {
		_inactiveToggleListener = _inactiveContent->onClick(
			[this]([[maybe_unused]] ref<const engine::input::event::MouseButtonEvent> event_) {
				// Warning: Maybe cause use-after-free due to immediate tree-rewrite in `setValue`
				setValue(true);
				updateValueAndValidity(true, true);
				return EventResponse::eConsumed;
			}
		);
	}
}

InputToggle::value_type InputToggle::value() const noexcept {
	return getDataAttributes().valueOf<attr::InputData::value>();
}

void InputToggle::setValue(bool enabled_) {
	if (not getDataAttributes().update<attr::InputData::value>(enabled_)) { return; }

	if (enabled_) {
		_activeContent->setParent(shared_from_this());
		auto prev = _children.setChild(clone(_activeContent));
		prev->setParent(nullptr);
	} else {
		_inactiveContent->setParent(shared_from_this());
		auto prev = _children.setChild(clone(_inactiveContent));
		prev->setParent(nullptr);
	}

	markAsPending();
}

void InputToggle::updateValueAndValidity(bool propagate_, bool emit_) {
	discard = _emitter.emit(EditorUiEventNames::InputChangeEvent, InputChangeEvent { shared_from_this() });
	markAsPristine();
}

Widget::listen_handle_type InputToggle::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::InputChangeEvent, hg::move(listenFn_));
}

void InputToggle::onAttachDataWatcher(ref<DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.attach(this, getDataAttributes().get<attr::InputData::value>());
	}
	Input::onAttachDataWatcher(watcher_);
}
void InputToggle::onDetachDataWatcher(ref<DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.detach(getDataAttributes().get<attr::InputData::value>());
	}
	Input::onDetachDataWatcher(watcher_);
}

/**/

SharedPtr<InputToggle> editor::ui::makeInputToggle(
	mref<SharedPtr<Widget>> active_,
	mref<SharedPtr<Widget>> inactive_,
	bool value_
) {

	auto result = make_sptr<InputToggle>();
	result->setActiveContent(hg::move(active_));
	result->setInactiveContent(hg::move(inactive_));
	result->setValue(value_);

	return result;
}
