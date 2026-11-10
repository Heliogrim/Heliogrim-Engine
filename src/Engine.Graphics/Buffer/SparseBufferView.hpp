#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "BufferLikeObject.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx {
	class SparseBufferView final :
		public InheritMeta<SparseBufferView, BufferLikeObject> {
	public:
		friend class ::hg::engine::gfx::SparseBuffer;

	public:
		using this_type = SparseBufferView;

	protected:
		SparseBufferView(
			ref<SparseBuffer> owner_,
			mref<Vector<nmpt<SparseBufferPage>>> pages_,
			const u64 offset_,
			const u64 size_
		);

	public:
		SparseBufferView (cref<this_type>) = delete;

		SparseBufferView(mref<this_type>) noexcept = delete;

	public:
		~SparseBufferView() override;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) noexcept = delete;

	private:
		ref<SparseBuffer> _owner;

	public:
		[[nodiscard]] ref<const SparseBuffer> owner() const noexcept;

	private:
		Vector<nmpt<SparseBufferPage>> _pages;

	public:
		// TODO: Should be protected / private ...
		[[nodiscard]] ref<const Vector<nmpt<SparseBufferPage>>> pages() const noexcept;

	private:
		u64 _offset;
		u64 _size;

	public:
		[[nodiscard]] u64 offset() const noexcept;

		[[nodiscard]] u64 size() const noexcept;
	};
}