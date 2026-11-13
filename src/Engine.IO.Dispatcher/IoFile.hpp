#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::engine::io {
	class IoFile final :
		public ArcFromThis<IoFile> {
	public:
		// Takes ownership of an overlapped file HANDLE.
		explicit IoFile(ptr<void> handle_) noexcept;

		~IoFile();

	public:
		[[nodiscard]] ptr<void> handle() const noexcept;

	private:
		ptr<void> _handle;
	};
}
