#include "MaterialTransformer.hpp"

#include <ranges>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX.Material/MaterialFactory.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>

#include "../Texture/TextureResource.hpp"
#include "../Texture/Traits.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] static smr<TextureResource> resolveTexture(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

[[nodiscard]] static smr<MaterialPrototypeResource> resolvePrototype(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

/**/

MaterialTransformer::MaterialTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

MaterialTransformer::response_type::type MaterialTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    // TODO: Consume source data to hydrate the instantiated resource object
    auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});

    const auto& loader = engine::Engine::getEngine()->getResources()->loader();
    const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();

    /**/

    const auto protoRes = resolvePrototype(request_->prototype(), registry, &loader);
    const auto protoGuard = protoRes->acquire(resource::ResourceUsageFlag::eRead);

    /**/

    material::MaterialFactory factory {};
    auto fmat = factory.buildMaterial(clone(protoGuard->instance));

    using derived_type = ::hg::engine::resource::UniqueResource<MaterialResource::value_type>;
    auto dst = make_smr<MaterialResource, derived_type>(
        new derived_type(fmat.release())
    );

    auto materialGuard = dst->acquire(resource::ResourceUsageFlag::eAll);
    auto& material = *materialGuard.mut();

    /**/

    const auto count = protoGuard->instance->getParameters().size();
    for (size_t idx = 0; idx < count; ++idx) {

        const auto& proto = protoGuard->instance->getParameters()[idx];
        auto& param = const_cast<material::MaterialParameter&>(material.getParameters()[idx]);

        switch (proto.getDataType()) {
            case acc::TransferDataType::eConstant:
            case acc::TransferDataType::eUniform:
            case acc::TransferDataType::eStorage: break;
            case acc::TransferDataType::eSampler: {

                asset_guid guid {};
                auto resolved = resolveTexture(_STD move(guid), registry, &loader);

                param.set(_STD move(resolved));
                break;
            }
            default: {}
        }

    }

    /**
     * Cleanup and Return
     */

    dst->setAssociation(request_);

    return dst;
}

smr<engine::gfx::TextureResource> resolveTexture(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

    /**
     * Resolve texture asset from database
     */
    auto* const asset = registry_->findAssetByGuid(guid_);

    #ifdef _DEBUG
    assert(asset != nullptr);

    if (!asset->getMetaClass()->exact<engine::assets::TextureAsset>()) {
        __debugbreak();
    }
    #endif

    /**
     * Load texture asset to get the internal resource handle
     *  // TODO: This should be a non-residential load operation, cause we only need the handle
     */
    //auto* textureAsset = Cast<engine::assets::Texture, engine::assets::Asset, false>(asset);
    auto* textureAsset = static_cast<ptr<engine::assets::TextureAsset>>(asset);
    auto texture = loader_->loadImmediately<engine::assets::TextureAsset, TextureResource>(
        _STD move(textureAsset),
        TextureLoadOptions {
            textureAsset->getExtent().x >= 8192 ?
                TextureLoadDataFlagBits::eLazyDataLoading :
                TextureLoadDataFlagBits::eNone
        }
    );

    /**/

    return texture;
}

smr<MaterialPrototypeResource> resolvePrototype(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

    /**
     * Resolve material prototype asset from database
     */
    auto* const asset = registry_->findAssetByGuid(guid_);

    /**
     * Load material prototype to get the internal resource handle
     */
    //auto* matProtAsset = Cast<engine::assets::GfxMaterialPrototype, engine::assets::Asset, false>(asset);
    auto* matProtAsset = static_cast<ptr<engine::assets::GfxMaterialPrototype>>(asset);
    auto prototype = loader_->loadImmediately<engine::assets::GfxMaterialPrototype, MaterialPrototypeResource>(
        _STD move(matProtAsset),
        {}
    );

    /**/

    return prototype;
}
