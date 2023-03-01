#include "StaticGeometryFeedback.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

StaticGeometryFeedback::StaticGeometryFeedback() {}

StaticGeometryFeedback::response_type::type StaticGeometryFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_));
}
