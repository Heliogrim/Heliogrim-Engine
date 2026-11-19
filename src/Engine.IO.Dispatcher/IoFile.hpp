#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::io {
	class IoFile final :
		public ArcFromThis<IoFile> {
	public:
		// Takes ownership of an overlapped file HANDLE.
		explicit IoFile(ptr<void> handle_) noexcept;

		~IoFile();

	private:
		ptr<void> _handle;

	public:
		[[nodiscard]] ptr<void> handle() const noexcept;
	};
}
