#include "IoThread.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::io;
using namespace hg;

IoThread::IoThread() :
	_thread("IO Thread", async::AffineWaker {}) {}

IoThread::~IoThread() {
	::hg::assertd(not _running);
	if (_port != nullptr) {
		::CloseHandle(_port);
	}
}

void IoThread::start(mref<harvest_type> harvest_) {
	::hg::assertd(_port == nullptr);
	_harvest = std::move(harvest_);
	_running = true;
	_port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
	::hg::assertrt(_port != nullptr);

	_thread.start(
		[this, port = _port](ref<async::AffineThread> self_) {
			self_.setWaker(
				[port] {
					::PostQueuedCompletionStatus(port, 0, wake_key, nullptr);
				}
			);

			OVERLAPPED_ENTRY entries[64];
			for (;;) {
				self_.drain();
				if (self_.stopping()) {
					self_.drain();
					return;
				}

				ULONG count = 0;
				if (not ::GetQueuedCompletionStatusEx(port, entries, 64, &count, INFINITE, FALSE)) {
					IM_CORE_ERRORF("IO thread wait failed: {}", ::GetLastError());
					::hg::assertd(false);
					continue;
				}

				for (ULONG i = 0; i < count; ++i) {
					if (entries[i].lpCompletionKey == wake_key) {
						continue;
					}
					::hg::assertd(static_cast<bool>(_harvest));
					if (_harvest) {
						_harvest(reinterpret_cast<const IoPortEntry&>(entries[i]));
					}
				}
			}
		}
	);
}

void IoThread::stop() {
	_thread.stop();
	_running = false;
}

ref<hg::engine::async::AffineThread> IoThread::thread() noexcept {
	return _thread;
}

ptr<void> IoThread::port() const noexcept {
	return _port;
}
