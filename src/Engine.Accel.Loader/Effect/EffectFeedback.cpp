#include "EffectFeedback.hpp"

using namespace hg::engine::accel::loader;
using namespace hg;

EffectFeedback::EffectFeedback() :
    Feedback() {}

EffectFeedback::response_type::type EffectFeedback::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
