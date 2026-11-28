#include "IoSystem.hpp"

#include <Engine.Common/stdafx.h>

#include "FileOps.hpp"

import Engine.Asserts;
import Engine.Async;

using namespace hg::engine::io;
using namespace hg;

IoSystem::IoSystem(const IoBackendKind kind_, const IoLimits limits_) :
	_thread(),
	_backend(
		IoBackend::make(
			kind_,
			_thread,
			[this](const ptr<void> token_, const IoCompletion completion_) {
				--_inFlight;
				_inFlightBytes -= IoQueue::requestOf(token_).buffer.size();
				IoQueue::complete(token_, completion_);
			}
		)
	),
	_limits() {
	setLimits(limits_);
}

IoSystem::~IoSystem() {
	stop();
}

void IoSystem::setLimits(const IoLimits limits_) {
	::hg::assertrt(not _started);
	::hg::assertrt(limits_.maxInFlight > 0u && limits_.maxInFlightBytes > 0u);
	_limits = limits_;
}

void IoSystem::start() {
	_started = true;
	_running = true;
	_thread.start([this](ref<const IoPortEntry> entry_) { _backend->harvest(entry_); }, [this] { return pump(); });
}

void IoSystem::stop() {
	if (not _running.exchange(false)) {
		return;
	}

	// shortcut: a submit posted after the inbox closes breaks its promise; callers stop producing before stop().
	_thread.thread().postDetached(
		[this] {
			_stopping = true;
			for (const auto& list : _active) {
				for (const auto& queue : list) {
					queue->cancelMatching(0uLL, 0uLL);
				}
			}
			_backend->cancelAll();
		}
	);
	_drained.wait(false);
	_thread.stop();
}

bool IoSystem::running() const noexcept {
	return _running;
}

u64 IoSystem::requestsIssued() const noexcept {
	return _requestsIssued.load(std::memory_order_relaxed);
}

Future<Arci<IoFile>, IoError> IoSystem::open(ref<const std::filesystem::path> path_, const IoOpenMode mode_) {
	auto promise = Promise<Arci<IoFile>, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[this, path = path_, mode_, promise = ::hg::move(promise)]() mutable {
			const auto unbuffered = (static_cast<u8>(mode_) & static_cast<u8>(IoOpenMode::eUnbuffered)) != 0u;
			const auto rw = (static_cast<u8>(mode_) & ~static_cast<u8>(IoOpenMode::eUnbuffered)) ==
				static_cast<u8>(IoOpenMode::eReadWrite);
			// Unbuffered writes would need aligned callers; mutations stay buffered.
			if (unbuffered && rw) {
				promise.reject(IoError { ERROR_NOT_SUPPORTED });
				return;
			}

			const auto create = [&] {
				return ::CreateFileW(
					path.c_str(),
					rw ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					nullptr,
					rw ? OPEN_ALWAYS : OPEN_EXISTING,
					_backend->openFlags() | (unbuffered ? FILE_FLAG_NO_BUFFERING : 0u),
					nullptr
				);
			};

			auto handle = create();
			// Like LfsSyncBlob: a new package may live in a directory that does not exist yet; only pay for it then.
			if (handle == INVALID_HANDLE_VALUE && rw && ::GetLastError() == ERROR_PATH_NOT_FOUND) {
				std::error_code ec {};
				std::filesystem::create_directories(path.parent_path(), ec);
				handle = create();
			}
			if (handle == INVALID_HANDLE_VALUE) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}

			// The file closes the handle on every path from here.
			// shortcut: 4096 covers 512e and 4Kn sectors; query FileAlignmentInfo if a device ever needs more.
			auto file = Arci<IoFile>::create(handle, unbuffered ? 4096u : 1u);
			if (const auto error = _backend->associate(handle); error != 0u) {
				promise.reject(IoError { error });
				return;
			}

			promise.resolve(::hg::move(file));
		}
	);
	return future;
}

Arci<IoQueue> IoSystem::createQueue(const IoPriority priority_) {
	return Arci<IoQueue>::create(*this, priority_);
}

Future<u64, IoError> IoSystem::size(Arci<IoFile> file_) {
	auto promise = Promise<u64, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = ::hg::move(file_), promise = ::hg::move(promise)]() mutable {
			LARGE_INTEGER value {};
			if (not::GetFileSizeEx(file->handle(), &value)) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}

			promise.resolve(static_cast<u64>(value.QuadPart));
		}
	);
	return future;
}

Future<void, IoError> IoSystem::resize(Arci<IoFile> file_, const u64 size_) {
	auto promise = Promise<void, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = ::hg::move(file_), size_, promise = ::hg::move(promise)]() mutable {
			FILE_END_OF_FILE_INFO info {};
			info.EndOfFile.QuadPart = static_cast<LONGLONG>(size_);
			if (not::SetFileInformationByHandle(file->handle(), FileEndOfFileInfo, &info, sizeof(info))) {
				promise.reject(IoError { ::GetLastError() });
				return;
			}

			promise.resolve();
		}
	);
	return future;
}

Future<IoTemp, IoError> IoSystem::openTemp(ref<const std::filesystem::path> target_) {
	auto promise = Promise<IoTemp, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[this, target = target_, promise = ::hg::move(promise)]() mutable {
			auto temp = createTempSibling(target, _backend->openFlags());
			if (not temp.has_value()) {
				promise.reject(IoError { temp.error().osError });
				return;
			}

			// The file closes the handle on every path from here.
			auto file = Arci<IoFile>::create(temp->handle, 1u);
			if (const auto error = _backend->associate(temp->handle); error != 0u) {
				std::error_code ec {};
				std::filesystem::remove(temp->path, ec);
				promise.reject(IoError { error });
				return;
			}

			promise.resolve(IoTemp { ::hg::move(file), ::hg::move(temp->path) });
		}
	);
	return future;
}

Future<void, IoError> IoSystem::flush(const Arci<IoFile> file_) {
	auto promise = Promise<void, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = file_, promise = ::hg::move(promise)]() mutable {
			if (const auto error = flushFile(file->handle()); error != 0u) {
				promise.reject(IoError { error });
				return;
			}

			promise.resolve();
		}
	);
	return future;
}

Future<void, IoError> IoSystem::renameOver(const Arci<IoFile> file_, const std::filesystem::path target_) {
	auto promise = Promise<void, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[file = file_, target = target_, promise = ::hg::move(promise)]() mutable {
			if (const auto error = ::hg::engine::io::renameOver(file->handle(), target); error != 0u) {
				promise.reject(IoError { error });
				return;
			}

			promise.resolve();
		}
	);
	return future;
}

Future<void, IoError> IoSystem::replaceByName(const std::filesystem::path temp_, const std::filesystem::path target_) {
	auto promise = Promise<void, IoError> {};
	auto future = promise.future();

	_thread.thread().postDetached(
		[temp = temp_, target = target_, promise = ::hg::move(promise)]() mutable {
			if (const auto error = ::hg::engine::io::replaceByName(temp, target); error != 0u) {
				promise.reject(IoError { error });
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
		_active[static_cast<size_t>(queue_->priority())].push_back(::hg::move(queue_));
	}
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

bool IoSystem::saturated() const noexcept {
	return _inFlight >= _limits.maxInFlight || _inFlightBytes >= _limits.maxInFlightBytes;
}

bool IoSystem::pump() {
	if (_stopping) {
		if (_inFlight == 0u && not _drained) {
			_drained = true;
			_drained.notify_all();
		}
		return false;
	}

	// Bounded, so a synchronous backend returns to the inbox between rounds.
	for (u32 issued = 0; issued < _limits.maxInFlight && not saturated(); ++issued) {
		const auto token = pick();
		if (token == nullptr) {
			return false;
		}

		const auto& request = IoQueue::requestOf(token);
		++_inFlight;
		_inFlightBytes += request.buffer.size();
		_requestsIssued.fetch_add(1, std::memory_order_relaxed);
		_backend->issue(request, token);
	}
	return not saturated();
}
