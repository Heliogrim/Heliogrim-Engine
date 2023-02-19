#include "MaterialTransformer.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

MaterialTransformer::MaterialTransformer() :
    Transformer() {}

MaterialTransformer::response_type::type MaterialTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});
    return response_type::type {};
}
