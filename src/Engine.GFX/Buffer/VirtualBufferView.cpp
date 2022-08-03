#include "VirtualBufferView.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualBufferView::VirtualBufferView(
    non_owning_rptr<VirtualBuffer> owner_,
    mref<Vector<non_owning_rptr<VirtualBufferPage>>> pages_,
    const u64 offset_,
    const u64 size_
):
    _owner(owner_),
    _pages(_STD move(pages_)),
    _offset(offset_),
    _size(size_) {}

VirtualBufferView::~VirtualBufferView() = default;

const non_owning_rptr<const VirtualBuffer> VirtualBufferView::owner() const noexcept {
    return _owner;
}

u64 VirtualBufferView::offset() const noexcept {
    return _offset;
}

u64 VirtualBufferView::size() const noexcept {
    return _size;
}
