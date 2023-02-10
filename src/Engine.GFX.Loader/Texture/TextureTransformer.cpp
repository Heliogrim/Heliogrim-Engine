#include "TextureTransformer.hpp"

#include <Engine.GFX/Pool/GlobalResourcePool.hpp>

#include "./Transformer/Ktx.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

smr<engine::gfx::TextureResource> TextureTransformer::makeEmptyResource() const {
    return make_smr<TextureResource>();
}

smr<TextureResource> TextureTransformer::transpose(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    mref<smr<resource::Source>> from_,
    mref<smr<TextureResource>> to_
) const {

    auto dst = _STD move(to_);
    auto guard = dst->acquire(resource::ResourceUsageFlag::eAll);

    /**/

    const non_owning_rptr<const assets::Texture> asset = request_;
    auto view = _pool->requestVirtualTexture(asset);

    /**/

    transformer::convertKtx(asset, from_, view.get());

    /**/

    *guard.mut() = _STD move(view);
    guard.release();

    /**/

    return dst;
}

smr<TextureResource> TextureTransformer::partialTranspose(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_,
    mref<smr<resource::Source>> from_,
    mref<smr<TextureResource>> to_
) const {
    return to_;
}

TextureTransformer::response_type::type TextureTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    auto dst = makeEmptyResource();
    auto src = next_({}, next_type::next_request_type::options {});

    return transpose(_STD move(request_), _STD move(options_), _STD move(src), _STD move(dst));
}
