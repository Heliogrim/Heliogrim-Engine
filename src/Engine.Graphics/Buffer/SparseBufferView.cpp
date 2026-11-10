#include "SparseBufferView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseBufferView::SparseBufferView(
	ref<SparseBuffer> owner_,
	mref<Vector<nmpt<SparseBufferPage>>> pages_,
	const u64 offset_,
	const u64 size_
) :
	_owner(owner_),
	_pages(std::move(pages_)),
	_offset(offset_),
	_size(size_) {}

SparseBufferView::~SparseBufferView() = default;

ref<const SparseBuffer> SparseBufferView::owner() const noexcept {
	return _owner;
}

ref<const Vector<nmpt<SparseBufferPage>>> SparseBufferView::pages() const noexcept {
	return _pages;
}

u64 SparseBufferView::offset() const noexcept {
	return _offset;
}

u64 SparseBufferView::size() const noexcept {
	return _size;
}