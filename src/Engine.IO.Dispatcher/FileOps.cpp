#include "FileOps.hpp"

#include <Engine.Common/stdafx.h>

using namespace hg::engine::io;
using namespace hg;

namespace {
	constexpr u32 max_name_tries = 8u;

	[[nodiscard]] bool isTempOf(std::wstring_view name_, std::wstring_view stem_) noexcept;
}

#if ENV_WIN

Result<TempFile, IoError> hg::engine::io::createTempSibling(ref<const std::filesystem::path> target_, const u32 flags_) {
	std::random_device rd {};
	const auto stem = target_.filename().wstring();

	u32 error = ERROR_FILE_EXISTS;
	for (u32 attempt = 0u; attempt < max_name_tries; ++attempt) {
		auto path = target_;
		path.replace_filename(std::format(L"{}.{:08x}{}", stem, static_cast<u32>(rd()), temp_suffix));

		// DELETE: the handle itself renames the temp over the target.
		const auto handle = ::CreateFileW(
			path.c_str(),
			GENERIC_READ | GENERIC_WRITE | DELETE,
			FILE_SHARE_READ | FILE_SHARE_DELETE,
			nullptr,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL | flags_,
			nullptr
		);
		if (handle != INVALID_HANDLE_VALUE) {
			return Result<TempFile, IoError>::expected(TempFile { handle, std::move(path) });
		}

		error = ::GetLastError();
		if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS) {
			break;
		}
	}
	return Result<TempFile, IoError>::unexpected(IoError { error });
}

u32 hg::engine::io::flushFile(const ptr<void> handle_) noexcept {
	return ::FlushFileBuffers(handle_) ? 0u : ::GetLastError();
}

u32 hg::engine::io::renameOver(const ptr<void> handle_, ref<const std::filesystem::path> target_) noexcept {
	const auto& name = target_.native();
	const auto nameBytes = name.size() * sizeof(wchar_t);

	if (nameBytes > std::numeric_limits<DWORD>::max() - sizeof(FILE_RENAME_INFO)) {
		return ERROR_FILENAME_EXCED_RANGE;
	}

	// Element type guarantees alignment; the name is copied behind the fixed part (FileName[1] holds the terminator).
	Vector<FILE_RENAME_INFO> buffer {};
	try {
		buffer.resize((sizeof(FILE_RENAME_INFO) + nameBytes + sizeof(FILE_RENAME_INFO) - 1uz) / sizeof(FILE_RENAME_INFO));
	} catch (...) {
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	auto& info = buffer.front();
	info.Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS;
	info.RootDirectory = nullptr;
	info.FileNameLength = static_cast<DWORD>(nameBytes);
	std::memcpy(info.FileName, name.data(), nameBytes);

	return ::SetFileInformationByHandle(
			handle_,
			FileRenameInfoEx,
			buffer.data(),
			static_cast<DWORD>(buffer.size() * sizeof(FILE_RENAME_INFO))
		) ?
		0u :
		::GetLastError();
}

u32 hg::engine::io::replaceByName(ref<const std::filesystem::path> temp_, ref<const std::filesystem::path> target_) noexcept {
	// A failed MoveFileExW leaves the target intact, unlike ReplaceFileW which can fail after removing it.
	return ::MoveFileExW(temp_.c_str(), target_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) ? 0u : ::GetLastError();
}

void hg::engine::io::sweepStaleTemps(ref<const std::filesystem::path> target_) noexcept {
	std::error_code ec {};
	const auto stem = target_.filename().wstring();
	auto dir = target_.parent_path();
	if (dir.empty()) {
		dir = ".";
	}

	for (std::filesystem::directory_iterator it { dir, ec }; not ec && it != std::filesystem::directory_iterator {};
	     it.increment(ec)) {
		if (isTempOf(it->path().filename().native(), stem)) {
			// Only an exclusive open succeeds, so a temp another commit still holds is left alone.
			const auto handle = ::CreateFileW(it->path().c_str(), DELETE, 0u, nullptr, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
			if (handle != INVALID_HANDLE_VALUE) {
				::CloseHandle(handle);
			}
		}
	}
}

#else

// shortcut: POSIX replace is W03.14, these stubs fail closed (callers map to eUnsupportedVolume) until fsync and rename land.
Result<TempFile, IoError> hg::engine::io::createTempSibling(ref<const std::filesystem::path>, u32) {
	return Result<TempFile, IoError>::unexpected(IoError { error_not_supported });
}

u32 hg::engine::io::flushFile(ptr<void>) noexcept {
	return error_not_supported;
}

u32 hg::engine::io::renameOver(ptr<void>, ref<const std::filesystem::path>) noexcept {
	return error_not_supported;
}

u32 hg::engine::io::replaceByName(ref<const std::filesystem::path>, ref<const std::filesystem::path>) noexcept {
	return error_not_supported;
}

void hg::engine::io::sweepStaleTemps(ref<const std::filesystem::path>) noexcept {}

#endif

/**/

namespace {
	[[nodiscard]] bool isTempOf(const std::wstring_view name_, const std::wstring_view stem_) noexcept {
		// `<stem>.` + 8 lowercase hex + `.hgtmp`
		if (name_.size() != stem_.size() + 1uz + 8uz + temp_suffix.size()) {
			return false;
		}

		if (not name_.starts_with(stem_) || name_[stem_.size()] != L'.' || not name_.ends_with(temp_suffix)) {
			return false;
		}

		const auto hex = name_.substr(stem_.size() + 1uz, 8uz);
		return std::ranges::all_of(
			hex,
			[](const wchar_t c_) { return (c_ >= L'0' && c_ <= L'9') || (c_ >= L'a' && c_ <= L'f'); }
		);
	}
}
