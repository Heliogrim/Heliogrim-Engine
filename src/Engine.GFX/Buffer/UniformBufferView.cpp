#include "UniformBufferView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

nmpt<BufferLikeObject> UniformBufferView::object() const noexcept {
    return _blo;
}

void UniformBufferView::storeBuffer(mref<nmpt<BufferLikeObject>> object_) noexcept {
    _blo = _STD move(object_);
}
