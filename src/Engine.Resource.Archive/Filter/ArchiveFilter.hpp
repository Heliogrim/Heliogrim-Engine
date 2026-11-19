#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class macro_novtable ArchiveFilter {
	public:
		using this_type = ArchiveFilter;

	protected:
		ArchiveFilter() = default;

	public:
		virtual ~ArchiveFilter() = default;

	private:
		ptr<ArchiveFilter> _next;

	public:
		[[nodiscard]] const ptr<ArchiveFilter> next() const noexcept;

		void setNext(const ptr<ArchiveFilter> next_) noexcept;
	};
}
