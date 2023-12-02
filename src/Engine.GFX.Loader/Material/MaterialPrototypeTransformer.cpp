#include "MaterialPrototypeTransformer.hpp"

#include <ranges>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/AccelEffect.hpp>
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

#include <Engine.Accel.Loader/Effect/EffectResource.hpp>
#include <Engine.Accel.Loader/Effect/Traits.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] static smr<TextureResource> resolveTexture(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

[[nodiscard]] static smr<const engine::accel::AccelerationEffect> resolveEffect(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

/**/

MaterialPrototypeTransformer::MaterialPrototypeTransformer(const non_owning_rptr<pool::GlobalResourcePool> pool_) :
    Transformer(),
    _pool(pool_) {}

MaterialPrototypeTransformer::response_type::type MaterialPrototypeTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    // TODO: Consume source data to hydrate the instantiated resource object
    //auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});

    const auto& loader = engine::Engine::getEngine()->getResources()->loader();
    const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();

    /**/

    material::MaterialFactory factory {};

    Vector<material::MaterialPrototypeParameter> params {};
    InlineAutoArray<material::MaterialEffect> effects {};

    for (const auto& serialParam : request_->_params) {

        auto dst = factory.buildMaterialPrototypeParameter(clone(serialParam.uniqueName), clone(serialParam.dataType));
        if (serialParam.defaultValue.valueless_by_exception()) {
            params.push_back(_STD move(dst));
            continue;
        }

        switch (dst.getDataType()) {
            case accel::TransferDataType::eConstant:
            case accel::TransferDataType::eUniform:
            case accel::TransferDataType::eStorage: break;
            case accel::TransferDataType::eSampler: {

                assert(_STD holds_alternative<asset_guid>(serialParam.defaultValue));

                auto textureGuid = _STD get<asset_guid>(serialParam.defaultValue);
                auto resolved = resolveTexture(_STD move(textureGuid), registry, &loader);
                dst.set(_STD move(resolved));
                break;
            }
            case accel::TransferDataType::eU8: {
                assert(_STD holds_alternative<u8>(serialParam.defaultValue));
                dst.set(_STD get<u8>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU16: {
                assert(_STD holds_alternative<u16>(serialParam.defaultValue));
                dst.set(_STD get<u16>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU32: {
                assert(_STD holds_alternative<u32>(serialParam.defaultValue));
                dst.set(_STD get<u32>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU64: {
                assert(_STD holds_alternative<u64>(serialParam.defaultValue));
                dst.set(_STD get<u64>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32: {
                assert(_STD holds_alternative<float>(serialParam.defaultValue));
                dst.set(_STD get<float>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU8Vec2: {
                assert(_STD holds_alternative<math::vec2_t<u8>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec2_t<u8>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU16Vec2: {
                assert(_STD holds_alternative<math::vec2_t<u16>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec2_t<u16>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU32Vec2: {
                assert(_STD holds_alternative<math::vec2_t<u32>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec2_t<u32>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU64Vec2: {
                assert(_STD holds_alternative<math::vec2_t<u64>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec2_t<u64>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32Vec2: {
                assert(_STD holds_alternative<math::vec2_t<float>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec2_t<float>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU8Vec3: {
                assert(_STD holds_alternative<math::vec3_t<u8>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec3_t<u8>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU16Vec3: {
                assert(_STD holds_alternative<math::vec3_t<u16>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec3_t<u16>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU32Vec3: {
                assert(_STD holds_alternative<math::vec3_t<u32>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec3_t<u32>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU64Vec3: {
                assert(_STD holds_alternative<math::vec3_t<u64>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec3_t<u64>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32Vec3: {
                assert(_STD holds_alternative<math::vec3_t<float>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec3_t<float>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU8Vec4: {
                assert(_STD holds_alternative<math::vec4_t<u8>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec4_t<u8>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU16Vec4: {
                assert(_STD holds_alternative<math::vec4_t<u16>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec4_t<u16>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU32Vec4: {
                assert(_STD holds_alternative<math::vec4_t<u32>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec4_t<u32>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eU64Vec4: {
                assert(_STD holds_alternative<math::vec4_t<u64>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec4_t<u64>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32Vec4: {
                assert(_STD holds_alternative<math::vec4_t<u32>>(serialParam.defaultValue));
                dst.set(_STD get<math::vec4_t<u32>>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32Mat3x3: {
                assert(_STD holds_alternative<math::matq3_t<float>>(serialParam.defaultValue));
                dst.set(_STD get<u8>(serialParam.defaultValue));
                break;
            }
            case accel::TransferDataType::eF32Mat4x4: {
                assert(_STD holds_alternative<math::matq4_t<float>>(serialParam.defaultValue));
                dst.set(_STD get<math::matq4_t<float>>(serialParam.defaultValue));
                break;
            }
            default: {
                throw _STD runtime_error("Unexpected.");
            }
        }

        params.push_back(_STD move(dst));
    }

    /**/

    for (auto effect : request_->_effects) {

        auto resolved = resolveEffect(_STD move(effect), registry, &loader);
        effects.emplace_back(_STD move(resolved), nullptr);
    }

    /**
     * Cleanup and Return
     */

    using derived_type = ::hg::engine::resource::UniqueResource<MaterialPrototypeResource::value_type>;
    auto dst = make_smr<derived_type>(
        MaterialPrototypeWrapper {
            make_smr<material::MaterialPrototype>(
                Guid { request_->get_guid() },
                string { request_->getAssetName() },
                _STD move(effects),
                _STD move(params)
            )
        }
    );

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

smr<const engine::accel::AccelerationEffect> resolveEffect(
    mref<asset_guid> guid_,
    const non_owning_rptr<const engine::assets::IAssetRegistry> registry_,
    const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

    /**
     * Resolve effect asset from database
     */
    auto* const asset = registry_->findAssetByGuid(guid_);

    /**
     * Load material prototype to get the internal resource handle
     */
    auto* effectAsset = static_cast<ptr<engine::assets::AccelEffect>>(asset);
    auto loaded = loader_->loadImmediately<engine::assets::AccelEffect, engine::accel::EffectResource>(
        _STD move(effectAsset),
        {}
    );

    auto guard = loaded->acquire(engine::resource::ResourceUsageFlag::eRead);
    return guard.imm()->instance;
}
