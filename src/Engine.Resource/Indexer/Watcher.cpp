#include "Watcher.hpp"

#include <assert.h>
#include <filesystem>
#include <thread>

using namespace ember::engine::res;
using namespace ember;

Watcher::Watcher(cref<File> root_) :
    _root(root_) {
    setup();
}

Watcher::~Watcher() {
    tidy();
}

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
        _handle = NULL;
    }

}

void Watcher::setup() {

    constexpr auto mask = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE |
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

    /**
     * Setup Notification Handle
     */
    _handle = FindFirstChangeNotification(_root.url().c_str(), TRUE, mask);
    assert(_handle != INVALID_HANDLE_VALUE);

    /**
     * Submit Waiting
     */
    // WaitForSingleObject(_handle, 500000);
    // WaitForMultipleObjects(1, &_handle, FALSE, 500000);
    RegisterWaitForSingleObject(&_waitHandle, _handle, [](IN PVOID self_, IN BOOLEAN timerOrWait_) {

        /**
         * Guard from timeout
         */
        if (timerOrWait_ == TRUE) {
            return;
        }

        static_cast<ptr<Watcher>>(self_)->notify();
        assert(FindNextChangeNotification(static_cast<ptr<Watcher>>(self_)->_handle));

    }, this, INFINITE, WT_EXECUTEDEFAULT);

    /**
     * Preregister first state
     */
    notify(false);
}

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

    for (const auto& entry : _STD filesystem::recursive_directory_iterator { _root.url() }) {

        const File file { entry.path().string() };
        const auto lastWrite = _STD filesystem::last_write_time(entry);

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

cref<File> Watcher::root() const noexcept {
    return _root;
}

File Watcher::root() noexcept {
    return _root;
}

Watcher::operator const File() const noexcept {
    return _root;
}

Watcher::operator File() noexcept {
    return _root;
}

void Watcher::setCreateCallback(cref<std::function<void(File file_)>> callback_) noexcept {
    _createCallback = callback_;
}

void Watcher::setModifiedCallback(cref<std::function<void(File file_)>> callback_) noexcept {
    _modifiedCallback = callback_;
}

void Watcher::setEraseCallback(cref<std::function<void(File file_)>> callback_) noexcept {
    _eraseCallback = callback_;
}
