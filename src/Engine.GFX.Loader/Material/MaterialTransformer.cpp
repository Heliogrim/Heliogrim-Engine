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
    //auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});

    const auto& loader = engine::Engine::getEngine()->getResources()->loader();
    const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();

    /**/

    assert(request_->prototype() != invalid_asset_guid);
    const auto protoRes = resolvePrototype(request_->prototype(), registry, &loader);
    const auto protoGuard = protoRes->acquire(resource::ResourceUsageFlag::eRead);

    /**/

    material::MaterialFactory factory {};
    auto fmat = factory.buildMaterial(clone(protoGuard->instance));

    using derived_type = ::hg::engine::resource::UniqueResource<MaterialResource::value_type>;
    auto result = make_smr<MaterialResource, derived_type>(
        new derived_type(fmat.release())
    );

    auto materialGuard = result->acquire(resource::ResourceUsageFlag::eAll);
    auto& material = *materialGuard.mut();

    /**/

    cref<Vector<material::MaterialPrototypeParameter>> protoParams = protoGuard->instance->getParameters();

    for (auto [protoParam, dstParam] : std::views::zip(
             protoParams,
             material.getParameters()
         )) {

        auto& dst = const_cast<material::MaterialParameter&>(dstParam);

        auto matParamIt = std::find_if(
            request_->_params.begin(),
            request_->_params.end(),
            [&protoParam](cref<assets::GfxMaterial::TmpParam> candidate_) {
                return candidate_.identifier == protoParam.getId();
            }
        );

        /**/

        if (matParamIt == request_->_params.end()) {

            switch (protoParam.getDataType()) {
                case accel::TransferDataType::eConstant:
                case accel::TransferDataType::eUniform:
                case accel::TransferDataType::eStorage: {
                    dst.set(clone(protoParam.getDefaultValue<smr<void>>()));
                    break;
                }
                case accel::TransferDataType::eSampler: {
                    dst.set(clone(protoParam.getDefaultValue<smr<TextureResource>>()));
                    break;
                }
                case accel::TransferDataType::eU8: {
                    dst.set(protoParam.getDefaultValue<u8>());
                    break;
                }
                case accel::TransferDataType::eU16: {
                    dst.set(protoParam.getDefaultValue<u16>());
                    break;
                }
                case accel::TransferDataType::eU32: {
                    dst.set(protoParam.getDefaultValue<u32>());
                    break;
                }
                case accel::TransferDataType::eU64: {
                    dst.set(protoParam.getDefaultValue<u64>());
                    break;
                }
                case accel::TransferDataType::eF32: {
                    dst.set(protoParam.getDefaultValue<float>());
                    break;
                }
                case accel::TransferDataType::eU8Vec2: {
                    dst.set(protoParam.getDefaultValue<math::vec2_t<u8>>());
                    break;
                }
                case accel::TransferDataType::eU16Vec2: {
                    dst.set(protoParam.getDefaultValue<math::vec2_t<u16>>());
                    break;
                }
                case accel::TransferDataType::eU32Vec2: {
                    dst.set(protoParam.getDefaultValue<math::vec2_t<u32>>());
                    break;
                }
                case accel::TransferDataType::eU64Vec2: {
                    dst.set(protoParam.getDefaultValue<math::vec2_t<u64>>());
                    break;
                }
                case accel::TransferDataType::eF32Vec2: {
                    dst.set(protoParam.getDefaultValue<math::vec2_t<float>>());
                    break;
                }
                case accel::TransferDataType::eU8Vec3: {
                    dst.set(protoParam.getDefaultValue<math::vec3_t<u8>>());
                    break;
                }
                case accel::TransferDataType::eU16Vec3: {
                    dst.set(protoParam.getDefaultValue<math::vec3_t<u16>>());
                    break;
                }
                case accel::TransferDataType::eU32Vec3: {
                    dst.set(protoParam.getDefaultValue<math::vec3_t<u32>>());
                    break;
                }
                case accel::TransferDataType::eU64Vec3: {
                    dst.set(protoParam.getDefaultValue<math::vec3_t<u64>>());
                    break;
                }
                case accel::TransferDataType::eF32Vec3: {
                    dst.set(protoParam.getDefaultValue<math::vec3_t<float>>());
                    break;
                }
                case accel::TransferDataType::eU8Vec4: {
                    dst.set(protoParam.getDefaultValue<math::vec4_t<u8>>());
                    break;
                }
                case accel::TransferDataType::eU16Vec4: {
                    dst.set(protoParam.getDefaultValue<math::vec4_t<u16>>());
                    break;
                }
                case accel::TransferDataType::eU32Vec4: {
                    dst.set(protoParam.getDefaultValue<math::vec4_t<u32>>());
                    break;
                }
                case accel::TransferDataType::eU64Vec4: {
                    dst.set(protoParam.getDefaultValue<math::vec4_t<u64>>());
                    break;
                }
                case accel::TransferDataType::eF32Vec4: {
                    dst.set(protoParam.getDefaultValue<math::vec4_t<float>>());
                    break;
                }
                case accel::TransferDataType::eF32Mat3x3: {
                    dst.set(protoParam.getDefaultValue<math::matq3_t<float>>());
                    break;
                }
                case accel::TransferDataType::eF32Mat4x4: {
                    dst.set(protoParam.getDefaultValue<math::matq4_t<float>>());
                    break;
                }
                default: {
                    throw std::runtime_error("Unexpected.");
                }

            }

            continue;
        }

        /**/

        const auto& matParam = *matParamIt;
        assert(matParam.dataType == protoParam.getDataType());

        switch (matParam.dataType) {
            case accel::TransferDataType::eConstant:
            case accel::TransferDataType::eUniform:
            case accel::TransferDataType::eStorage: break;
            case accel::TransferDataType::eSampler: {

                assert(std::holds_alternative<asset_guid>(matParam.value));

                auto textureGuid = std::get<asset_guid>(matParam.value);
                auto resolved = resolveTexture(std::move(textureGuid), registry, &loader);

                dst.set(std::move(resolved));
                break;
            }
            case accel::TransferDataType::eU8: {
                assert(std::holds_alternative<u8>(matParam.value));
                dst.set(std::get<u8>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU16: {
                assert(std::holds_alternative<u16>(matParam.value));
                dst.set(std::get<u16>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU32: {
                assert(std::holds_alternative<u32>(matParam.value));
                dst.set(std::get<u32>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU64: {
                assert(std::holds_alternative<u64>(matParam.value));
                dst.set(std::get<u64>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32: {
                assert(std::holds_alternative<float>(matParam.value));
                dst.set(std::get<float>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU8Vec2: {
                assert(std::holds_alternative<math::vec2_t<u8>>(matParam.value));
                dst.set(std::get<math::vec2_t<u8>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU16Vec2: {
                assert(std::holds_alternative<math::vec2_t<u16>>(matParam.value));
                dst.set(std::get<math::vec2_t<u16>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU32Vec2: {
                assert(std::holds_alternative<math::vec2_t<u32>>(matParam.value));
                dst.set(std::get<math::vec2_t<u32>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU64Vec2: {
                assert(std::holds_alternative<math::vec2_t<u64>>(matParam.value));
                dst.set(std::get<math::vec2_t<u64>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32Vec2: {
                assert(std::holds_alternative<math::vec2_t<float>>(matParam.value));
                dst.set(std::get<math::vec2_t<float>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU8Vec3: {
                assert(std::holds_alternative<math::vec3_t<u8>>(matParam.value));
                dst.set(std::get<math::vec3_t<u8>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU16Vec3: {
                assert(std::holds_alternative<math::vec3_t<u16>>(matParam.value));
                dst.set(std::get<math::vec3_t<u16>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU32Vec3: {
                assert(std::holds_alternative<math::vec3_t<u32>>(matParam.value));
                dst.set(std::get<math::vec3_t<u32>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU64Vec3: {
                assert(std::holds_alternative<math::vec3_t<u64>>(matParam.value));
                dst.set(std::get<math::vec3_t<u64>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32Vec3: {
                assert(std::holds_alternative<math::vec3_t<float>>(matParam.value));
                dst.set(std::get<math::vec3_t<float>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU8Vec4: {
                assert(std::holds_alternative<math::vec4_t<u8>>(matParam.value));
                dst.set(std::get<math::vec4_t<u8>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU16Vec4: {
                assert(std::holds_alternative<math::vec4_t<u16>>(matParam.value));
                dst.set(std::get<math::vec4_t<u16>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU32Vec4: {
                assert(std::holds_alternative<math::vec4_t<u32>>(matParam.value));
                dst.set(std::get<math::vec4_t<u32>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eU64Vec4: {
                assert(std::holds_alternative<math::vec4_t<u64>>(matParam.value));
                dst.set(std::get<math::vec4_t<u64>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32Vec4: {
                assert(std::holds_alternative<math::vec4_t<u32>>(matParam.value));
                dst.set(std::get<math::vec4_t<u32>>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32Mat3x3: {
                assert(std::holds_alternative<math::matq3_t<float>>(matParam.value));
                dst.set(std::get<u8>(matParam.value));
                break;
            }
            case accel::TransferDataType::eF32Mat4x4: {
                assert(std::holds_alternative<math::matq4_t<float>>(matParam.value));
                dst.set(std::get<math::matq4_t<float>>(matParam.value));
                break;
            }
            default: {
                throw std::runtime_error("Unexpected.");
            }
        }

    }

    /**
     * Cleanup and Return
     */

    result->setAssociation(request_);
    return result;
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
        std::move(textureAsset),
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
        std::move(matProtAsset),
        {}
    );

    /**/

    return prototype;
}
