#include "TextureFeedback.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

TextureFeedback::TextureFeedback() = default;

TextureFeedback::traits::response_value_type TextureFeedback::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_
) const {
    return {};
}

TextureFeedback::traits::response_value_type TextureFeedback::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_,
    mref<traits::stream_options_type> streamOptions_
) const {
    return {};
}
