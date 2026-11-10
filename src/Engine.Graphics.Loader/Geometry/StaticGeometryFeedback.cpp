#include "StaticGeometryFeedback.hpp"

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg;

StaticGeometryFeedback::StaticGeometryFeedback() {}

StaticGeometryFeedback::response_type::type StaticGeometryFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
