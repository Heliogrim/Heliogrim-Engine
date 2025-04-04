#include "MaterialPrototypeFeedback.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

MaterialPrototypeFeedback::MaterialPrototypeFeedback() :
    Feedback() {}

MaterialPrototypeFeedback::response_type::type MaterialPrototypeFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
