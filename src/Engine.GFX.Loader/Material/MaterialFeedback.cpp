#include "MaterialFeedback.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

MaterialFeedback::MaterialFeedback() :
    Feedback() {}

MaterialFeedback::response_type::type MaterialFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
