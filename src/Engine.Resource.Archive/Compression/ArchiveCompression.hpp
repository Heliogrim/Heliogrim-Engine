#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class macro_novtable ArchiveCompression {
	public:
		using this_type = ArchiveCompression;

	protected:
		ArchiveCompression() = default;

	public:
		virtual ~ArchiveCompression() = default;

	private:
		ptr<ArchiveCompression> _next;

	public:
		[[nodiscard]] const ptr<ArchiveCompression> next() const noexcept;

		void setNext(const ptr<ArchiveCompression> next_) noexcept;
	};
}
