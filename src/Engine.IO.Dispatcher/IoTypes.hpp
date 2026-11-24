#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::io {
	class IoFile;

	enum class IoOp : u8 {
		eRead,
		eWrite
	};

	enum class IoCompression : u8 {
		eNone
	};

	enum class IoPriority : u8 {
		eLow,
		eNormal,
		eHigh,
		eRealtime
	};

	enum class IoStatus : u8 {
		eDone,
		eShort,
		eCancelled,
		eFailed
	};

	// Flag bit: combined with eRead only; reads then bypass the OS cache and must be aligned (IoFile::alignment).
	enum class IoOpenMode : u8 {
		eRead,
		eReadWrite,
		eUnbuffered = 0x80
	};

	struct IoError {
		u32 osError;
	};

	// Base for objects whose lifetime must span a request (e.g. an owned buffer); derive and hold an Arci to it.
	class IoKeepAlive :
		public ArcFromThis<IoKeepAlive> {
	public:
		virtual ~IoKeepAlive() = default;
	};

	struct IoRequest {
		IoOp op;
		Arci<IoFile> file;
		u64 offset;
		std::span<_::byte> buffer;
		IoCompression compression = IoCompression::eNone;
		u64 tag = 0;
		// Held by the batch until the request completes, so a dropped future with an owned buffer stays safe.
		Arci<IoKeepAlive> keepAlive = {};
	};

	// In-flight bounds of an io system; the byte budget is checked before each issue, so one request may overshoot it.
	struct IoLimits {
		u32 maxInFlight = 32u;
		u32 maxInFlightBytes = 2u * 1024u * 1024u;
	};

	struct IoCompletion {
		IoStatus status;
		u32 bytes;
		u32 osError;
	};
}
