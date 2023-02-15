#include "TextureTransformer.hpp"

#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>

#include "./Transformer/Ktx.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

TextureTransformer::TextureTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

smr<TextureResource> TextureTransformer::transpose(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    mref<smr<resource::Source>> from_
) const {

    const non_owning_rptr<const assets::Texture> asset = request_;
    auto view = _pool->allocateVirtualTexture({ asset });

    /**/

    transformer::convertKtx(asset, from_, view.get(), options_);

    /**/

    using derived_type = resource::UniqueResource<TextureResource::value_type>;

    auto dst = make_smr<TextureResource, derived_type>(
        new derived_type(_STD move(view.release()))
    );

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
    auto src = next_({}, next_type::next_request_type::options {});
    return transpose(_STD move(request_), _STD move(options_), _STD move(src));
}

engine::resource::loader::CacheStreamResponse<engine::assets::Texture>::type TextureTransformer::operator()(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_,
    cref<next_type> next_
) const {
    // TODO:
    const auto source = next_({}, next_type::next_stream_request_type::options {});
    // return (void);
}
