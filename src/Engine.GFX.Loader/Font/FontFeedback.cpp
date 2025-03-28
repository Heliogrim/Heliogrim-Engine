#include "FontFeedback.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

FontFeedback::FontFeedback() :
    Feedback() {}

FontFeedback::response_type::type FontFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
