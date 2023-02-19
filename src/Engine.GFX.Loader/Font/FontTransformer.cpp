#include "FontTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

FontTransformer::FontTransformer() :
    Transformer() {}

FontTransformer::response_type::type FontTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});
    return {};
}
