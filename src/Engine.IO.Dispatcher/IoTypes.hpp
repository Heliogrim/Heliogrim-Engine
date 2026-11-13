#pragma once

#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

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

	// Flag bit: combined with eRead or eReadWrite.
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

	struct IoCompletion {
		IoStatus status;
		u32 bytes;
		u32 osError;
	};
}
