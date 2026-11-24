#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::io {
	class IoFile final :
		public ArcFromThis<IoFile> {
	public:
		// Takes ownership of an overlapped file HANDLE; alignment_ is 1 or the unbuffered sector alignment.
		explicit IoFile(const ptr<void> handle_, const u32 alignment_) noexcept;

		~IoFile();

	private:
		ptr<void> _handle;
		u32 _alignment;

	public:
		[[nodiscard]] ptr<void> handle() const noexcept;

		// Offsets, lengths and buffer addresses of requests on this file must be multiples of it.
		[[nodiscard]] u32 alignment() const noexcept;
	};
}
