#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::resource {
	class __declspec(novtable) ArchiveFilter {
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
