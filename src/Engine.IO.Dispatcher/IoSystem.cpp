#include "IoSystem.hpp"

#include <Engine.Common/stdafx.h>

using namespace hg::engine::io;
using namespace hg;

IoSystem::IoSystem(const IoBackendKind kind_, const u32 maxInFlight_) :
	_thread(),
	_backend(
		IoBackend::make(
			kind_,
			_thread,
			[this](const ptr<void> token_, const IoCompletion completion_) {
				--_inFlight;
				IoQueue::complete(token_, completion_);
			}
		)
	),
	_maxInFlight(maxInFlight_) {}

IoSystem::~IoSystem() {
	stop();
}

void IoSystem::start() {
	_running = true;
	_thread.start(
		[this](ref<const IoPortEntry> entry_) {
			_backend->harvest(entry_);
			pump();
		}
	);
}

void IoSystem::stop() {
	// shortcut: in-flight io is not drained, its slots leak and batches stay pending; Task 2 (StopDrainsInFlight).
	if (_running.exchange(false)) {
		_thread.stop();
	}
}

bool IoSystem::running() const noexcept {
	return _running;
}

Future<Arci<IoFile>, IoError> IoSystem::open(cref<std::filesystem::path> path_, const IoOpenMode mode_) {
	Promise<Arci<IoFile>, IoError> promise {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[this, path = path_, mode_, promise = std::move(promise)]() mutable {
			if ((static_cast<u8>(mode_) & static_cast<u8>(IoOpenMode::eUnbuffered)) != 0u) {
				promise.reject(IoError { ERROR_NOT_SUPPORTED });
				return;
			}

			const auto rw = mode_ == IoOpenMode::eReadWrite;
			const auto handle = ::CreateFileW(
				path.c_str(),
				rw ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr,
				rw ? OPEN_ALWAYS : OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				nullptr
			);
			if (handle == INVALID_HANDLE_VALUE) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}

			// The file closes the handle on every path from here.
			auto file = Arci<IoFile>::create(handle);
			if (const auto error = _backend->associate(handle); error != 0u) {
				promise.reject(IoError { error });
				return;
			}
			promise.resolve(std::move(file));
		}
	);
	return future;
}

Arci<IoQueue> IoSystem::createQueue(const IoPriority priority_) {
	return Arci<IoQueue>::create(*this, priority_);
}

Future<u64, IoError> IoSystem::size(Arci<IoFile> file_) {
	Promise<u64, IoError> promise {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = std::move(file_), promise = std::move(promise)]() mutable {
			LARGE_INTEGER value {};
			if (not ::GetFileSizeEx(file->handle(), &value)) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}
			promise.resolve(static_cast<u64>(value.QuadPart));
		}
	);
	return future;
}

Future<void, IoError> IoSystem::resize(Arci<IoFile> file_, const u64 size_) {
	Promise<void, IoError> promise {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = std::move(file_), size_, promise = std::move(promise)]() mutable {
			FILE_END_OF_FILE_INFO info {};
			info.EndOfFile.QuadPart = static_cast<LONGLONG>(size_);
			if (not ::SetFileInformationByHandle(file->handle(), FileEndOfFileInfo, &info, sizeof(info))) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}
			promise.resolve();
		}
	);
	return future;
}

ref<hg::engine::async::AffineThread> IoSystem::thread() noexcept {
	return _thread.thread();
}

void IoSystem::activate(mref<Arci<IoQueue>> queue_) {
	if (not queue_->_listed) {
		queue_->_listed = true;
		_active[static_cast<size_t>(queue_->priority())].push_back(std::move(queue_));
	}
	pump();
}

ptr<void> IoSystem::pick() {
	for (auto level = _active.size(); level-- > 0;) {
		auto& list = _active[level];
		for (size_t k = 0; k < list.size(); ++k) {
			const auto idx = (_cursor[level] + k) % list.size();
			if (const auto token = list[idx]->next(); token != nullptr) {
				_cursor[level] = idx + 1;
				return token;
			}
		}

		// Nothing left to issue at this level; queues re-list on their next submit.
		for (auto& queue : list) {
			queue->_listed = false;
		}
		list.clear();
		_cursor[level] = 0;
	}
	return nullptr;
}

void IoSystem::pump() {
	while (_inFlight < _maxInFlight) {
		const auto token = pick();
		if (token == nullptr) {
			break;
		}
		++_inFlight;
		_backend->issue(IoQueue::requestOf(token), token);
	}
}
