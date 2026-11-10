#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "BufferLikeObject.hpp"

namespace hg::engine::gfx {
	class StorageBufferView :
		public InheritBase<StorageBufferView> {
	public:
		using this_type = StorageBufferView;

	public:
		constexpr StorageBufferView() noexcept = default;

		constexpr StorageBufferView(nmpt<BufferLikeObject> view_) noexcept :
			InheritBase(),
			_blo(view_) {}

		constexpr StorageBufferView(ref<const this_type>) noexcept = default;

		constexpr StorageBufferView(mref<this_type>) noexcept = default;

		constexpr ~StorageBufferView() noexcept override = default;

	private:
		nmpt<BufferLikeObject> _blo;

	public:
		[[nodiscard]] nmpt<BufferLikeObject> object() const noexcept;

		void storeBuffer(mref<nmpt<BufferLikeObject>> object_) noexcept;
	};
}
