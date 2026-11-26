#include "ReplaceSequence.hpp"

#include <Engine.Common/__macro.hpp>

#if ENV_WIN
#include <Engine.Common/stdafx.h>
#endif

import Engine.Common;
import Engine.IO.Dispatcher;
import Engine.Scheduler;
import std;

using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

namespace {
	// shortcut: a fiber yields until the deadline (re-queued each turn, so its worker runs others); swap for a timed park once the scheduler has one.
	void backoff(const std::chrono::milliseconds wait_) {
		if (scheduler::fiber::self::current() == nullptr) {
			std::this_thread::sleep_for(wait_);
			return;
		}
		const auto until = std::chrono::steady_clock::now() + wait_;
		while (std::chrono::steady_clock::now() < until) {
			scheduler::fiber::self::yield();
		}
	}

	[[nodiscard]] bool isReadOnlyFile(ref<const std::filesystem::path> path_) noexcept {
		#if ENV_WIN
		const auto attributes = ::GetFileAttributesW(path_.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY) != 0u;
		#else
		return false;
		#endif
	}

	[[nodiscard]] Result<void, io::CommitError> failure(
		ref<const std::filesystem::path> target_,
		const io::CommitError::Kind kind_,
		const u32 osError_,
		const u32 retriedMs_ = 0u
	) {
		return Result<void, io::CommitError>::unexpected(
			io::CommitError { .kind = kind_, .path = pathText(target_), .osError = osError_, .retriedMs = retriedMs_ }
		);
	}
}

Opt<io::CommitError> hg::engine::resource::checkReplaceSpace(ref<const std::filesystem::path> target_, const streamsize size_) {

	auto dir = target_.parent_path();
	if (dir.empty()) {
		dir = ".";
	}

	std::error_code ec {};
	const auto info = std::filesystem::space(dir, ec);
	if (ec) {
		return SomeVal<io::CommitError>(
			{
				.kind = io::commitKindOf(static_cast<u32>(ec.value())), .path = pathText(target_), .osError = static_cast<u32>(ec.value())
			}
		);
	}

	const auto size = static_cast<u64>((std::max)(size_, streamsize { 0 }));
	const auto needed = size + (std::max)(1uLL << 20uLL, size / 64uLL);
	if (info.available < needed) {
		return SomeVal<io::CommitError>({ .kind = io::CommitError::Kind::eDiskFull, .path = pathText(target_), .bytesNeeded = needed });
	}
	return None;
}

String hg::engine::resource::pathText(ref<const std::filesystem::path> path_) {
	String out {};
	for (const auto c : path_.u8string()) {
		out.push_back(static_cast<char>(c));
	}
	return out;
}

Result<void, io::CommitError> hg::engine::resource::runReplaceSteps(
	ref<const std::filesystem::path> target_,
	ref<const ReplaceSteps> steps_,
	ref<bool> swapped_
) {

	// A failed flush is never retried; the caller's unfinished temp is removed on drop.
	if (const auto error = steps_.flushTemp(); error != 0u) {
		return failure(target_, io::CommitError::Kind::eFlushFailed, error);
	}

	auto error = forceByNameReplace.load() ? io::error_not_supported : steps_.renameTemp();
	u32 retriedMs = 0u;
	// A read-only target never opens up by waiting; it may turn read-only between tries.
	const auto readOnly = [&]() { return error == io::error_access_denied && isReadOnlyFile(target_); };
	// About 2 s worst case on a held target.
	for (const auto sleepMs : { 100u, 200u, 400u, 600u, 700u }) {
		if (readOnly()) {
			return failure(target_, io::CommitError::Kind::eReadOnly, error, retriedMs);
		}

		if (error != io::error_sharing_violation && error != io::error_lock_violation && error != io::error_access_denied) {
			break;
		}

		backoff(std::chrono::milliseconds { sleepMs });
		retriedMs += sleepMs;
		error = steps_.renameTemp();
	}
	if (readOnly()) {
		return failure(target_, io::CommitError::Kind::eReadOnly, error, retriedMs);
	}

	bool byName = false;
	if (error == io::error_invalid_parameter || error == io::error_not_supported) {
		byName = true;
		if (const auto replaced = steps_.replaceByName(); replaced != 0u) {
			return failure(target_, io::CommitError::Kind::eUnsupportedVolume, replaced, retriedMs);
		}

		error = 0u;
	}

	if (error != 0u) {
		return failure(target_, io::commitKindOf(error), error, retriedMs);
	}

	// The package is new from here on; a failed flush says so but cannot undo it.
	swapped_ = true;
	if (const auto flushed = steps_.flushTarget(byName); flushed != 0u) {
		auto failed = failure(target_, io::CommitError::Kind::eFlushFailed, flushed, retriedMs);
		failed.error().replaced = true;
		return failed;
	}

	return Result<void, io::CommitError>::expected();
}

#if ENV_WIN

u32 hg::engine::resource::flushByName(ref<const std::filesystem::path> target_) noexcept {
	const auto handle = ::CreateFileW(
		target_.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (handle == INVALID_HANDLE_VALUE) {
		return ::GetLastError();
	}

	const auto error = io::flushFile(handle);
	::CloseHandle(handle);
	return error;
}

#else

u32 hg::engine::resource::flushByName(ref<const std::filesystem::path>) noexcept {
	return io::error_not_supported;
}

#endif
