#include "Watcher.hpp"

#include <cassert>
#include <filesystem>
#include <thread>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::engine::res;
using namespace hg;

Watcher::Watcher(cref<::hg::fs::File> root_) :
	_root(root_) {
	setup();
}

Watcher::~Watcher() {
	tidy();
}

#if defined(_WIN32)
void Watcher::tidy() {

	if (_handle) {
		/**
		 * Unregister Wait
		 */
		UnregisterWait(_waitHandle);

		/**
		 * Close Handle
		 */
		// FindCloseChangeNotification(_handle);
		_handle = nullptr;
	}

}

void Watcher::setup() {

	constexpr auto mask = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

	/**
	 * Setup Notification Handle
	 */
	_handle = FindFirstChangeNotification(static_cast<std::string>(_root.path()).c_str(), TRUE, mask);
	assert(_handle != INVALID_HANDLE_VALUE);

	/**
	 * Submit Waiting
	 */
	// WaitForSingleObject(_handle, 500000);
	// WaitForMultipleObjects(1, &_handle, FALSE, 500000);
	RegisterWaitForSingleObject(
		&_waitHandle,
		_handle,
		[](_In_ PVOID self_, _In_ BOOLEAN timerOrWait_) {

			/**
			 * Guard from timeout
			 */
			if (timerOrWait_ == TRUE) {
				return;
			}

			static_cast<ptr<Watcher>>(self_)->notify();
			#ifdef _DEBUG
            assert(FindNextChangeNotification(static_cast<ptr<Watcher>>(self_)->_handle));
			#else
			FindNextChangeNotification(static_cast<ptr<Watcher>>(self_)->_handle);
			#endif

		},
		this,
		INFINITE,
		WT_EXECUTEDEFAULT
	);

	/**
	 * Preregister first state
	 */
	notify(false);
}
#else

void Watcher::tidy() {
	::hg::todo_panic();
}

void Watcher::setup() {
	::hg::todo_panic();
}

#endif

void Watcher::notify(const bool publish_) {

	auto iter = _state.begin();
	while (iter != _state.end()) {

		if (!iter->first.exists()) {
			iter = _state.erase(iter);
			if (publish_ && _eraseCallback) {
				_eraseCallback(iter->first);
			}

		} else {
			++iter;
		}
	}

	for (const auto& entry : std::filesystem::recursive_directory_iterator { _root.path() }) {

		const ::hg::fs::File file { fs::Path { entry.path() } };
		const auto lastWrite = std::filesystem::last_write_time(entry);

		/**
		 * Check for new files
		 */
		if (!_state.contains(file)) {
			_state[file] = lastWrite;
			if (publish_ && _createCallback) {
				_createCallback(file);
			}

		} else if (_state[file] != lastWrite) {
			_state[file] = lastWrite;
			if (publish_ && _modifiedCallback) {
				_modifiedCallback(file);
			}

		}

	}
}

cref<fs::File> Watcher::root() const noexcept {
	return _root;
}

fs::File Watcher::root() noexcept {
	return _root;
}

Watcher::operator const ::hg::fs::File() const noexcept {
	return _root;
}

Watcher::operator ::hg::fs::File() noexcept {
	return _root;
}

void Watcher::setCreateCallback(
	cref<std::function<void(::hg::fs::File file_)>> callback_
) noexcept {
	_createCallback = callback_;
}

void Watcher::setModifiedCallback(
	cref<std::function<void(::hg::fs::File file_)>> callback_
) noexcept {
	_modifiedCallback = callback_;
}

void Watcher::setEraseCallback(
	cref<std::function<void(::hg::fs::File file_)>> callback_
) noexcept {
	_eraseCallback = callback_;
}
