#include "Allocator.hpp"

using namespace ember::engine::gfx;
using namespace ember;

u64 Allocator::align() const noexcept {
    return _align;
}
