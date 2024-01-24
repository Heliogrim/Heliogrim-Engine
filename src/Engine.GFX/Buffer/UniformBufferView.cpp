#include "UniformBufferView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

nmpt<const BufferLikeObject> UniformBufferView::object() const noexcept {
    return _blo;
}

void UniformBufferView::storeBuffer(mref<nmpt<const BufferLikeObject>> object_) noexcept {
    _blo = std::move(object_);
}
