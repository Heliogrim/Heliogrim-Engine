#pragma once

#include "VirtualMemoryPageState.hpp"
#include "../Memory/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
	class VirtualMemory;
}

namespace hg::engine::graphics {
	class VirtualMemoryPage {
	public:
		using this_type = VirtualMemoryPage;

	public:
		VirtualMemoryPage(const nmpt<VirtualMemory> owner_, const u64 offset_, const u64 size_);

		~VirtualMemoryPage();

	private:
		const nmpt<VirtualMemory> _owner;

	public:
		[[nodiscard]] const nmpt<VirtualMemory> owner() const noexcept;

	private:
		u64 _offset;

	public:
		[[nodiscard]] u64 offset() const noexcept;

	private:
		u64 _size;

	public:
		[[nodiscard]] u64 size() const noexcept;

	private:
		volatile VirtualMemoryPageState _state;

	public:
		[[nodiscard]] VirtualMemoryPageState state() const noexcept;

		//private:
	public:
		uptr<memory::AllocatedMemory> _memory;

	public:
		[[nodiscard]] nmpt<memory::AllocatedMemory> allocated() const noexcept;

	public:
		[[nodiscard]] memory::AllocationResult load();

		void unload();
	};
}
