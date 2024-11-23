#include "MaterialPrototypeTransformer.hpp"

#include <ranges>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Accel/AccelEffect.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
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
#include <Engine.GFX.Render.Predefined/Descriptions/SceneView.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] static smr<TextureResource> resolveTexture(
	mref<asset_guid> guid_,
	nmpt<const engine::assets::IAssetRegistry> registry_,
	const non_owning_rptr<const engine::resource::LoaderManager> loader_
);

[[nodiscard]] static smr<const engine::accel::AccelerationEffect> resolveEffect(
	mref<asset_guid> guid_,
	nmpt<const engine::assets::IAssetRegistry> registry_,
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
	const auto registry = engine::Engine::getEngine()->getAssets()->getRegistry();

	/**/

	material::MaterialFactory factory {};

	Vector<material::MaterialPrototypeParameter> params {};
	InlineAutoArray<material::MaterialEffect> effects {};

	for (const auto& serialParam : request_->_params) {

		auto dst = factory.buildMaterialPrototypeParameter(
			clone(serialParam.identifier),
			clone(serialParam.name),
			clone(serialParam.dataType)
		);
		if (serialParam.defaultValue.valueless_by_exception()) {
			params.push_back(std::move(dst));
			continue;
		}

		switch (dst.getDataType()) {
			case accel::TransferDataType::eConstant:
			case accel::TransferDataType::eUniform:
			case accel::TransferDataType::eStorage: break;
			case accel::TransferDataType::eSampler: {

				assert(std::holds_alternative<asset_guid>(serialParam.defaultValue));

				auto textureGuid = std::get<asset_guid>(serialParam.defaultValue);
				auto resolved = resolveTexture(std::move(textureGuid), registry, &loader);
				dst.set(std::move(resolved));
				break;
			}
			case accel::TransferDataType::eU8: {
				assert(std::holds_alternative<u8>(serialParam.defaultValue));
				dst.set(std::get<u8>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU16: {
				assert(std::holds_alternative<u16>(serialParam.defaultValue));
				dst.set(std::get<u16>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU32: {
				assert(std::holds_alternative<u32>(serialParam.defaultValue));
				dst.set(std::get<u32>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU64: {
				assert(std::holds_alternative<u64>(serialParam.defaultValue));
				dst.set(std::get<u64>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32: {
				assert(std::holds_alternative<float>(serialParam.defaultValue));
				dst.set(std::get<float>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU8Vec2: {
				assert(std::holds_alternative<math::vec2_t<u8>>(serialParam.defaultValue));
				dst.set(std::get<math::vec2_t<u8>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU16Vec2: {
				assert(std::holds_alternative<math::vec2_t<u16>>(serialParam.defaultValue));
				dst.set(std::get<math::vec2_t<u16>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU32Vec2: {
				assert(std::holds_alternative<math::vec2_t<u32>>(serialParam.defaultValue));
				dst.set(std::get<math::vec2_t<u32>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU64Vec2: {
				assert(std::holds_alternative<math::vec2_t<u64>>(serialParam.defaultValue));
				dst.set(std::get<math::vec2_t<u64>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32Vec2: {
				assert(std::holds_alternative<math::vec2_t<float>>(serialParam.defaultValue));
				dst.set(std::get<math::vec2_t<float>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU8Vec3: {
				assert(std::holds_alternative<math::vec3_t<u8>>(serialParam.defaultValue));
				dst.set(std::get<math::vec3_t<u8>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU16Vec3: {
				assert(std::holds_alternative<math::vec3_t<u16>>(serialParam.defaultValue));
				dst.set(std::get<math::vec3_t<u16>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU32Vec3: {
				assert(std::holds_alternative<math::vec3_t<u32>>(serialParam.defaultValue));
				dst.set(std::get<math::vec3_t<u32>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU64Vec3: {
				assert(std::holds_alternative<math::vec3_t<u64>>(serialParam.defaultValue));
				dst.set(std::get<math::vec3_t<u64>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32Vec3: {
				assert(std::holds_alternative<math::vec3_t<float>>(serialParam.defaultValue));
				dst.set(std::get<math::vec3_t<float>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU8Vec4: {
				assert(std::holds_alternative<math::vec4_t<u8>>(serialParam.defaultValue));
				dst.set(std::get<math::vec4_t<u8>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU16Vec4: {
				assert(std::holds_alternative<math::vec4_t<u16>>(serialParam.defaultValue));
				dst.set(std::get<math::vec4_t<u16>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU32Vec4: {
				assert(std::holds_alternative<math::vec4_t<u32>>(serialParam.defaultValue));
				dst.set(std::get<math::vec4_t<u32>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eU64Vec4: {
				assert(std::holds_alternative<math::vec4_t<u64>>(serialParam.defaultValue));
				dst.set(std::get<math::vec4_t<u64>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32Vec4: {
				assert(std::holds_alternative<math::vec4_t<u32>>(serialParam.defaultValue));
				dst.set(std::get<math::vec4_t<u32>>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32Mat3x3: {
				assert(std::holds_alternative<math::matq3_t<float>>(serialParam.defaultValue));
				dst.set(std::get<u8>(serialParam.defaultValue));
				break;
			}
			case accel::TransferDataType::eF32Mat4x4: {
				assert(std::holds_alternative<math::matq4_t<float>>(serialParam.defaultValue));
				dst.set(std::get<math::matq4_t<float>>(serialParam.defaultValue));
				break;
			}
			default: {
				throw std::runtime_error("Unexpected.");
			}
		}

		params.push_back(std::move(dst));
	}

	/**/

	for (auto effect : request_->_effects) {

		auto resolved = resolveEffect(std::move(effect), registry, &loader);
		auto usagePattern = make_smr<material::EffectUsagePattern>();

		// Warning: Temporary Assumption to fast-forward
		// TODO: Rework as serialized data parameter

		Vector<accel::StageInput> imports {};
		resolved->enumerateImportSymbols(imports);

		/* Iterate Parameters */

		for (const auto& param : params) {

			// Warning:
			auto composed = string { "mat-static-" + std::to_string(param.getId().data) };
			auto maySymId = accel::lang::SymbolId::from(std::move(composed));

			for (const auto& import : imports) {
				if (import.symbol->symbolId == maySymId) {
					usagePattern->st.pairs.emplace_back(param.getId(), std::move(maySymId));
					break;
				}
			}

		}

		/* Iterate contextuals */

		for (const auto& import : imports) {

			// Warning:
			constexpr auto viewId = accel::lang::SymbolId::from("view"sv);
			if (import.symbol->symbolId == viewId) {
				usagePattern->ctx.pairs.emplace_back(render::makeSceneViewDescription(), import.symbol->symbolId);
			}

		}

		effects.emplace_back(std::move(resolved), std::move(usagePattern));
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
				std::move(effects),
				std::move(params)
			)
		}
	);

	dst->setAssociation(request_);

	return dst;
}

smr<engine::gfx::TextureResource> resolveTexture(
	mref<asset_guid> guid_,
	nmpt<const engine::assets::IAssetRegistry> registry_,
	const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

	/**
	 * Resolve texture asset from database
	 */
	const auto asset = registry_->findAssetByGuid(guid_);
	::hg::assertrt(asset != None);

	#ifdef _DEBUG
	if (!(*asset)->getMetaClass()->exact<engine::assets::TextureAsset>()) {
		::hg::breakpoint();
	}
	#endif

	/**
	 * Load texture asset to get the internal resource handle
	 *  // TODO: This should be a non-residential load operation, cause we only need the handle
	 */
	//auto* textureAsset = Cast<engine::assets::Texture, engine::assets::Asset, false>(asset);
	auto* textureAsset = Cast<engine::assets::TextureAsset>(asset->get());
	auto texture = loader_->loadImmediately<engine::assets::TextureAsset, TextureResource>(
		clone(textureAsset),
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
	nmpt<const engine::assets::IAssetRegistry> registry_,
	const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

	/**
	 * Resolve material prototype asset from database
	 */
	const auto asset = registry_->findAssetByGuid(guid_);
	::hg::assertrt(asset != None);

	/**
	 * Load material prototype to get the internal resource handle
	 */
	//auto* matProtAsset = Cast<engine::assets::GfxMaterialPrototype, engine::assets::Asset, false>(asset);
	auto* matProtAsset = Cast<engine::assets::GfxMaterialPrototype>(asset->get());
	auto prototype = loader_->loadImmediately<engine::assets::GfxMaterialPrototype, MaterialPrototypeResource>(
		std::move(matProtAsset),
		{}
	);

	/**/

	return prototype;
}

smr<const engine::accel::AccelerationEffect> resolveEffect(
	mref<asset_guid> guid_,
	nmpt<const engine::assets::IAssetRegistry> registry_,
	const non_owning_rptr<const engine::resource::LoaderManager> loader_
) {

	/**
	 * Resolve effect asset from database
	 */
	const auto asset = registry_->findAssetByGuid(guid_);
	::hg::assertrt(asset != None);

	/**
	 * Load material prototype to get the internal resource handle
	 */
	auto* effectAsset = Cast<engine::assets::AccelEffect>(asset->get());
	auto loaded = loader_->loadImmediately<engine::assets::AccelEffect, engine::accel::EffectResource>(
		std::move(effectAsset),
		{}
	);

	auto guard = loaded->acquire(engine::resource::ResourceUsageFlag::eRead);
	return guard.imm()->instance;
}
