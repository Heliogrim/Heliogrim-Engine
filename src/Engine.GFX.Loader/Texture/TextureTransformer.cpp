#include "TextureTransformer.hpp"

#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>

#include "./Transformer/Ktx.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

TextureTransformer::TextureTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

smr<TextureResource> TextureTransformer::transpose(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    mref<smr<resource::Source>> from_
) const {

    const non_owning_rptr<const assets::Texture> asset = request_;
    auto view = _pool->allocateVirtualTexture(
        pool::VirtualTextureAllocation {
            .layers = 1ui32,
            .extent = asset->getExtent(),
            .format = asset->getTextureFormat(),
            .mipLevels = { 0ui32, asset->getMipLevelCount() - 1ui32 },
            .type = asset->getTextureType()
        }
    );

    /**/

    transformer::convertKtx(asset, from_, view.get(), _pool->device(), options_);

    /**/

    using derived_type = resource::UniqueResource<TextureResource::value_type>;

    auto dst = make_smr<TextureResource, derived_type>(
        new derived_type(_STD move(view.release()))
    );
    dst->setAssociation(asset);

    /**/

    return dst;
}

void TextureTransformer::partialTranspose(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_,
    mref<smr<resource::Source>> from_,
    mref<smr<TextureResource>> to_
) const {

    const auto* const asset = static_cast<non_owning_rptr<const assets::Texture>>(request_->getAssociation());

    /**/

    const auto guard = to_->acquire(resource::ResourceUsageFlag::eWrite);
    auto* const view = guard->as<VirtualTextureView>();

    assert(view);

    /**/

    transformer::convertKtxPartial(asset, from_, view, _pool->device(), options_);
}

TextureTransformer::response_type::type TextureTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    auto asset = static_cast<non_owning_rptr<const assets::Asset>>(request_);
    auto src = next_(_STD move(asset), next_type::next_request_type::options {});

    /**/

    return transpose(_STD move(request_), _STD move(options_), _STD move(src));
}

engine::resource::loader::CacheStreamResponse<engine::assets::Texture>::type TextureTransformer::operator()(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_,
    cref<next_type> next_
) const {

    auto asset = static_cast<non_owning_rptr<const assets::Asset>>(request_->getAssociation());
    auto src = next_(_STD move(asset), next_type::next_request_type::options {});

    /**/

    auto cpy = request_;
    partialTranspose(_STD move(request_), _STD move(options_), _STD move(src), cpy.into<TextureResource>());
    // return (void);
}
