#include "TextureTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

TextureTransformer::traits::response_value_type TextureTransformer::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_
) const {
    return {};
}

TextureTransformer::traits::response_value_type TextureTransformer::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_,
    mref<traits::stream_options_type> streamOptions_
) const {
    return {};
}
