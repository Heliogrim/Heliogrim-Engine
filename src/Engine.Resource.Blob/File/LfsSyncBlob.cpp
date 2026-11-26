#include "LfsSyncBlob.hpp"

#include <Engine.Common/__macro.hpp>

#if ENV_WIN
#include <Engine.Common/stdafx.h>
#endif

#include "ReplaceSequence.hpp"

import Engine.Asserts;
import Engine.Async;
import Engine.Common;
import Engine.Reflect;
import std;
import Engine.IO.Dispatcher;

using namespace hg::engine::resource;
using namespace hg;
namespace io = hg::engine::io;

#if ENV_WIN

namespace {
	// Win32 lengths fit DWORD; larger transfers run in windows of this size.
	constexpr size_t max_window = 1uz << 30uz;

	[[nodiscard]] OVERLAPPED at(const u64 offset_) noexcept {
		OVERLAPPED ov {};
		ov.Offset = static_cast<DWORD>(offset_ & 0xFFFFFFFFuLL);
		ov.OffsetHigh = static_cast<DWORD>(offset_ >> 32uLL);
		return ov;
	}

	[[nodiscard]] HANDLE openShared(ref<const std::filesystem::path> path_, const DWORD access_, const DWORD disposition_) noexcept {
		return ::CreateFileW(
			path_.c_str(),
			access_,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			disposition_,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
	}

	[[nodiscard]] streamsize sizeOf(const ptr<void> handle_) noexcept {
		LARGE_INTEGER size {};
		return ::GetFileSizeEx(handle_, &size) ? static_cast<streamsize>(size.QuadPart) : streamsize { -1LL };
	}
}

/**/

LfsSyncBlob::LfsSyncBlob(mref<fs::Path> path_) noexcept :
	InheritMeta(),
	_path(path_) {

	const auto& path = static_cast<ref<const std::filesystem::path>>(_path);

	std::error_code ec {};
	if (not std::filesystem::exists(path, ec)) {
		std::filesystem::create_directories(path.parent_path(), ec);
	}

	_handle = openShared(path, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS);
	if (_handle == INVALID_HANDLE_VALUE) {
		_lastError = ::GetLastError();
		_handle = openShared(path, GENERIC_READ, OPEN_EXISTING);
		_readonly = true;
		if (_handle != INVALID_HANDLE_VALUE) {
			_lastError = 0u;
		}
	}

	::hg::assertrt(_handle != INVALID_HANDLE_VALUE);
}

LfsSyncBlob::LfsSyncBlob(ptr<void> tempHandle_, mref<fs::Path> tempPath_) noexcept :
	InheritMeta(),
	_path(tempPath_),
	_handle(tempHandle_),
	_temp(true) {}

LfsSyncBlob::~LfsSyncBlob() {
	if (_temp && not _finished) {
		std::error_code ec {};
		std::filesystem::remove(static_cast<ref<const std::filesystem::path>>(_path), ec);
	}
	if (_handle != nullptr && _handle != INVALID_HANDLE_VALUE) {
		::CloseHandle(_handle);
	}
}

std::span<_::byte> LfsSyncBlob::read(streamoff offset_, mref<std::span<_::byte>> dst_) const {
	concurrent::UniqueLock guard { _streamLock };

	if (_handle == nullptr) {
		_lastError = io::error_invalid_handle;
		return std::move(dst_).subspan(0uz, 0uz);
	}

	const auto filesize = sizeOf(_handle);
	::hg::assertrt(filesize >= 0LL && offset_ >= 0LL && offset_ <= filesize);

	const auto length = static_cast<size_t>((std::min)(filesize - offset_, static_cast<streamsize>(dst_.size())));

	size_t done = 0uz;
	while (done < length) {
		const auto window = static_cast<DWORD>((std::min)(length - done, max_window));
		auto ov = at(static_cast<u64>(offset_) + done);
		DWORD got = 0;
		if (not::ReadFile(_handle, dst_.data() + done, window, &got, &ov)) {
			_lastError = ::GetLastError();
			break;
		}

		done += got;
		if (got != window) {
			break;
		}
	}

	return std::move(dst_).subspan(0uz, done);
}

std::span<_::byte> LfsSyncBlob::write(streamoff offset_, mref<std::span<_::byte>> src_) {
	concurrent::UniqueLock guard { _streamLock };

	::hg::assertrt(not _readonly);
	if (_handle == nullptr) {
		_lastError = io::error_invalid_handle;
		return std::move(src_).subspan(0uz, 0uz);
	}

	const auto filesize = sizeOf(_handle);
	// Note: Guarantee that the offset is rewriting or directly appending without empty blocks in between.
	::hg::assertrt(filesize >= 0LL && offset_ >= 0LL && offset_ <= filesize);

	size_t done = 0uz;
	while (done < src_.size()) {
		const auto window = static_cast<DWORD>((std::min)(src_.size() - done, max_window));
		auto ov = at(static_cast<u64>(offset_) + done);
		DWORD put = 0;
		if (not::WriteFile(_handle, src_.data() + done, window, &put, &ov)) {
			_lastError = ::GetLastError();
			break;
		}

		done += put;
		if (put != window) {
			break;
		}
	}

	return std::move(src_).subspan(0uz, done);
}

bool LfsSyncBlob::trim(size_t tailSize_) {
	concurrent::UniqueLock guard { _streamLock };

	// Note: Only a writable blob can shrink its file.
	::hg::assertrt(not _readonly);
	if (_handle == nullptr) {
		_lastError = io::error_invalid_handle;
		return false;
	}

	if (tailSize_ <= 0) {
		return false;
	}

	const auto filesize = sizeOf(_handle);
	if (filesize < 0LL) {
		_lastError = ::GetLastError();
		return false;
	}

	FILE_END_OF_FILE_INFO info {};
	info.EndOfFile.QuadPart = filesize - static_cast<streamsize>((std::min)(static_cast<u64>(tailSize_), static_cast<u64>(filesize)));
	if (not::SetFileInformationByHandle(_handle, FileEndOfFileInfo, &info, sizeof(info))) {
		_lastError = ::GetLastError();
		return false;
	}

	return true;
}

Result<Rc<Blob>, io::CommitError> LfsSyncBlob::beginReplace(const streamsize size_) {

	const auto& target = static_cast<ref<const std::filesystem::path>>(_path);

	if (_handle == nullptr) {
		// A spent blob (after a replace) has no handle; reopen by path.
		return Result<Rc<Blob>, io::CommitError>::unexpected(
			io::CommitError { .path = pathText(target), .osError = io::error_invalid_handle }
		);
	}

	if (_readonly) {
		return Result<Rc<Blob>, io::CommitError>::unexpected(
			io::CommitError { .kind = io::CommitError::Kind::eReadOnly, .path = pathText(target) }
		);
	}

	if (auto full = checkReplaceSpace(target, size_); full.has_value()) {
		return Result<Rc<Blob>, io::CommitError>::unexpected(std::move(full).value());
	}

	io::sweepStaleTemps(target);

	auto temp = io::createTempSibling(target, 0u);
	if (not temp.has_value()) {
		_lastError = temp.error().osError;
		return Result<Rc<Blob>, io::CommitError>::unexpected(
			io::CommitError { .kind = io::commitKindOf(temp.error().osError), .path = pathText(target), .osError = temp.error().osError }
		);
	}

	return Result<Rc<Blob>, io::CommitError>::expected(
		Rc<LfsSyncBlob>::create(temp->handle, fs::Path { std::move(temp->path) })
	);
}

Result<void, io::CommitError> LfsSyncBlob::finishReplace(mref<Rc<Blob>> replacement_) {

	const auto repl = replacement_ != nullptr ? Cast<LfsSyncBlob>(replacement_.operator->()) : nullptr;
	if (repl == nullptr || not repl->_temp || repl->_finished) {
		return Result<void, io::CommitError>::unexpected(
			io::CommitError { .path = pathText(static_cast<ref<const std::filesystem::path>>(_path)) }
		);
	}

	const auto& target = static_cast<ref<const std::filesystem::path>>(_path);
	const auto& tempPath = static_cast<ref<const std::filesystem::path>>(repl->_path);

	const ReplaceSteps steps {
		.flushTemp = [repl]() { return io::flushFile(repl->_handle); },
		.renameTemp = [repl, &target]() { return io::renameOver(repl->_handle, target); },
		.replaceByName = [this, repl, &target, &tempPath]() {
			// MoveFileExW fails while any handle on either file is open; the blob is spent after the move.
			concurrent::UniqueLock guard { _streamLock };
			::CloseHandle(repl->_handle);
			repl->_handle = nullptr;
			if (_handle != nullptr) {
				::CloseHandle(_handle);
				_handle = nullptr;
			}
			return io::replaceByName(tempPath, target);
		},
		.flushTarget = [repl, &target](const bool byName_) {
			return byName_ ? flushByName(target) : io::flushFile(repl->_handle);
		}
	};

	auto result = runReplaceSteps(target, steps, repl->_finished);
	// The old handle points at the unlinked file; the blob is spent and callers reopen by path.
	if (repl->_finished) {
		concurrent::UniqueLock guard { _streamLock };
		if (_handle != nullptr) {
			::CloseHandle(_handle);
			_handle = nullptr;
		}
	}
	if (not result.has_value()) {
		_lastError = result.error().osError;
	}
	return result;
}

u32 LfsSyncBlob::lastError() const noexcept {
	return _lastError.load();
}

streamsize LfsSyncBlob::size() const noexcept {
	concurrent::UniqueLock guard { _streamLock };
	return _handle != nullptr ? sizeOf(_handle) : streamsize { -1LL };
}

#else

LfsSyncBlob::LfsSyncBlob(mref<fs::Path> path_) noexcept :
	InheritMeta(),
	_path(path_) {}

LfsSyncBlob::LfsSyncBlob(ptr<void>, mref<fs::Path> tempPath_) noexcept :
	InheritMeta(),
	_path(tempPath_) {}

LfsSyncBlob::~LfsSyncBlob() = default;

std::span<_::byte> LfsSyncBlob::read(streamoff, mref<std::span<_::byte>> dst_) const {
	return std::move(dst_).subspan(0uz, 0uz);
}

std::span<_::byte> LfsSyncBlob::write(streamoff, mref<std::span<_::byte>> src_) {
	return std::move(src_).subspan(0uz, 0uz);
}

bool LfsSyncBlob::trim(size_t) {
	return false;
}

Result<Rc<Blob>, io::CommitError> LfsSyncBlob::beginReplace(streamsize) {
	return Result<Rc<Blob>, io::CommitError>::unexpected(io::CommitError { .kind = io::CommitError::Kind::eUnsupportedVolume });
}

Result<void, io::CommitError> LfsSyncBlob::finishReplace(mref<Rc<Blob>>) {
	return Result<void, io::CommitError>::unexpected(io::CommitError { .kind = io::CommitError::Kind::eUnsupportedVolume });
}

streamsize LfsSyncBlob::size() const noexcept {
	return -1LL;
}

u32 LfsSyncBlob::lastError() const noexcept {
	return io::error_not_supported;
}

#endif
