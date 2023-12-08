#include "StorageBufferView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

nmpt<BufferLikeObject> StorageBufferView::object() const noexcept {
    return _blo;
}
