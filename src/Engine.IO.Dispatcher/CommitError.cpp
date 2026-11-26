#include "CommitError.hpp"

#include <Engine.Common/__macro.hpp>

using namespace hg::engine::io;
using namespace hg;

CommitError::Scope CommitError::scope() const noexcept {
	const auto volume = kind == Kind::eDiskFull || kind == Kind::eDeviceError || kind == Kind::eFlushFailed ||
		kind == Kind::eUnsupportedVolume;
	return volume ? Scope::eVolume : Scope::eFile;
}

CommitError::Kind hg::engine::io::commitKindOf(const u32 osError_) noexcept {
	#if ENV_WIN
	switch (osError_) {
		case error_disk_full:
		case error_handle_disk_full: {
			return CommitError::Kind::eDiskFull;
		}
		case error_sharing_violation:
		case error_lock_violation: {
			return CommitError::Kind::eLocked;
		}
		case error_access_denied: {
			return CommitError::Kind::eAccessDenied;
		}
		case error_write_protect: {
			return CommitError::Kind::eReadOnly;
		}
		case error_not_supported: {
			return CommitError::Kind::eUnsupportedVolume;
		}
		case error_crc:
		case error_io_device: {
			return CommitError::Kind::eDeviceError;
		}
		default: {
			return CommitError::Kind::eUnknown;
		}
	}
	#else
	return CommitError::Kind::eUnknown;
	#endif
}

StringView hg::engine::io::kindName(const CommitError::Kind kind_) noexcept {
	constexpr std::array<StringView, 9uz> names {
		"disk-full", "locked", "access-denied", "read-only", "unsupported-volume", "damaged-source", "device-error",
		"flush-failed", "unknown"
	};
	const auto index = static_cast<size_t>(kind_);
	return index < names.size() ? names[index] : names.back();
}
