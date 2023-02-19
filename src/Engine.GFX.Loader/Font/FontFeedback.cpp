#include "FontFeedback.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

FontFeedback::FontFeedback() :
    Feedback() {}

FontFeedback::response_type::type FontFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_));
}
