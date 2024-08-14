/* Note: Why-so-ever, this is order dependent... */
#include "Types/AccelEffect.hpp"
#include "Types/Asset.hpp"
#include "Types/Font.hpp"
#include "Types/Image.hpp"
#include "Types/LandscapeGeometry.hpp"
#include "Types/Level.hpp"
#include "Types/PfxMaterial.hpp"
#include "Types/SfxMaterial.hpp"
#include "Types/SkeletalGeometry.hpp"
#include "Types/Sound.hpp"
#include "Types/Geometry/StaticGeometry.hpp"
#include "Types/Material/GfxMaterial.hpp"
#include "Types/Material/GfxMaterialPrototype.hpp"
#include "Types/Texture/TextureAsset.hpp"
/* Note: Why-so-ever, this is order dependent... */

#include "AssetFactory.hpp"

#include <filesystem>
#include <sstream>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.System/Repository/InMemAssetRepository.hpp>
#include <Engine.Common/Meta/Type.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Meta/TypedMetaClass.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization.Layouts/LayoutManager.hpp>

using namespace hg::engine::assets;
using namespace hg;

static_assert(CompleteType<Asset>);
static_assert(CompleteType<Font>);
static_assert(CompleteType<GfxMaterialPrototype>);
static_assert(CompleteType<Image>);
static_assert(CompleteType<LandscapeGeometry>);
static_assert(CompleteType<Level>);
static_assert(CompleteType<PfxMaterial>);
static_assert(CompleteType<SfxMaterial>);
static_assert(CompleteType<SkeletalGeometry>);
static_assert(CompleteType<Sound>);
static_assert(CompleteType<StaticGeometry>);
static_assert(CompleteType<TextureAsset>);

/**/

void engine::assets::storeDefaultNameAndUrl(ref<Asset> asset_, string source_) {

	#ifdef _DEBUG
    if (not source_.empty() && asset_.getAssetName().empty()) {
        asset_.setAssetName(std::filesystem::path(source_).filename().string());
    }
	#endif

	if (asset_.getVirtualUrl().empty()) {

		std::filesystem::path pseudo {};

		switch (asset_.getTypeId().data) {
			case assets::Image::typeId.data: {
				pseudo = "texture";
				break;
			}
			case assets::TextureAsset::typeId.data: {
				pseudo = "texture";
				break;
			}
			case assets::GfxMaterial::typeId.data: {
				pseudo = "material";
				break;
			}
			case assets::StaticGeometry::typeId.data: {
				pseudo = "geometry";
				break;
			}
			case assets::Font::typeId.data: {
				pseudo = "font";
				break;
			}
			default: {
				pseudo = "content";
				break;
			}
		}

		if (not asset_.getAssetName().empty()) {

			std::stringstream ss {};
			ss << asset_.getAssetName();
			ss << " :: ";
			ss << asset_.get_guid().pre;
			ss << "-";
			ss << asset_.get_guid().c0;
			ss << "-";
			ss << asset_.get_guid().c1;
			ss << "-";
			ss << asset_.get_guid().post;

			pseudo /= ss.str();

		} else {

			std::stringstream ss {};
			ss << asset_.get_guid().pre;
			ss << "-";
			ss << asset_.get_guid().c0;
			ss << "-";
			ss << asset_.get_guid().c1;
			ss << "-";
			ss << asset_.get_guid().post;

			pseudo /= ss.str();
		}

		asset_.setVirtualUrl(pseudo.string());
	}

}

/**/

AssetFactory::AssetFactory(
	const non_owning_rptr<IAssetRegistry> registry_,
	const non_owning_rptr<system::InMemAssetRepository> repository_
) noexcept :
	_registry(registry_),
	_repository(repository_) {}

AssetFactory::~AssetFactory() noexcept = default;

fs::Url AssetFactory::resolveAsSource(cref<string> url_) const noexcept {

	auto cwd { std::filesystem::current_path() };
	cwd.append(url_);

	return fs::Url { "file"sv, cwd };
}

void AssetFactory::prepare() {

	using namespace ::hg::engine::serialization;

	auto& layouts { LayoutManager::get() };
	sptr<DataLayoutBase> cur { nullptr };

	/* Geometry */

	/* Material */

	/* Sound */

	/* Texture */
	cur = make_sptr<DataLayout<TextureAsset>>();
	cur->reflect().storeType<TextureAsset>();
	cur->describe();

	layouts.storeLayout("Assets::TextureAsset"sv, cur);
	layouts.storeLayout(TextureAsset::typeId, cur);
	layouts.storeLayout(TypedMetaClass<TextureAsset>::get(), cur);

	cur = make_sptr<DataLayout<Image>>();
	cur->reflect().storeType<Image>();
	cur->describe();

	layouts.storeLayout("Assets::Image"sv, cur);
	layouts.storeLayout(Image::typeId, cur);
	layouts.storeLayout(TypedMetaClass<Image>::get(), cur);
}

Arci<Font> AssetFactory::createFontAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<Font>::create(std::move(guid_), Vector<fs::Url> {});

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<Font> AssetFactory::createFontAsset(mref<asset_guid> guid_, cref<string> url_) const {

	auto src { resolveAsSource(url_) };
	Vector<fs::Url> sources {};

	if (src.hasScheme()/* Fast empty check */) {
		sources.push_back(src);
	}

	auto instance = Arci<Font>::create(std::move(guid_), std::move(sources));

	storeDefaultNameAndUrl(*instance, url_);
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<AccelEffect> AssetFactory::createAccelEffectAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<AccelEffect>::create(std::move(guid_));

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<GfxMaterial> AssetFactory::createGfxMaterialAsset(mref<asset_guid> prototypeGuid_) const {

	auto guid = generate_asset_guid();
	return createGfxMaterialAsset(std::move(guid), std::move(prototypeGuid_));
}

Arci<GfxMaterial> AssetFactory::createGfxMaterialAsset(
	mref<asset_guid> guid_,
	mref<asset_guid> prototypeGuid_
) const {

	auto instance = Arci<GfxMaterial>::create(std::move(guid_), std::move(prototypeGuid_));

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<GfxMaterialPrototype> AssetFactory::createGfxMaterialPrototypeAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<GfxMaterialPrototype>::create(std::move(guid_));

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<Image> AssetFactory::createImageAsset() const {

	auto guid = generate_asset_guid();
	return createImageAsset(std::move(guid));
}

Arci<Image> AssetFactory::createImageAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<Image>::create(std::move(guid_), Vector<fs::Url> {});

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<Image> AssetFactory::createImageAsset(mref<asset_guid> guid_, cref<string> url_) const {

	auto src { resolveAsSource(url_) };
	Vector<fs::Url> sources {};

	if (src.hasScheme()/* Fast empty check */) {
		sources.push_back(src);
	}

	auto instance = Arci<Image>::create(std::move(guid_), std::move(sources));

	storeDefaultNameAndUrl(*instance, String { src.path() });
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<LandscapeGeometry> AssetFactory::createLandscapeGeometryAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<LandscapeGeometry>::create(std::move(guid_), Vector<fs::Url> {});

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<StaticGeometry> AssetFactory::createStaticGeometryAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<StaticGeometry>::create(
		std::move(guid_),
		Vector<fs::Url> {},
		0uLL,
		0uLL
	);

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<StaticGeometry> AssetFactory::createStaticGeometryAsset(
	mref<asset_guid> guid_,
	cref<string> url_,
	cref<u64> vertexCount_,
	cref<u64> indexCount_
) const {

	auto src { resolveAsSource(url_) };
	Vector<fs::Url> sources {};

	if (src.hasScheme()/* Fast empty check */) {
		sources.push_back(src);
	}

	auto instance = Arci<StaticGeometry>::create(
		std::move(guid_),
		std::move(sources),
		vertexCount_,
		indexCount_
	);

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<TextureAsset> AssetFactory::createTextureAsset() const {

	auto guid = generate_asset_guid();
	return createTextureAsset(std::move(guid));
}

Arci<TextureAsset> AssetFactory::createTextureAsset(mref<asset_guid> guid_) const {

	auto instance = Arci<TextureAsset>::create(
		std::move(guid_),
		invalid_asset_guid,
		Vector<asset_guid> { invalid_asset_guid },
		math::uivec3 {},
		gfx::TextureFormat::eUndefined,
		0,
		gfx::TextureType::eUndefined
	);

	storeDefaultNameAndUrl(*instance, {});
	_registry->insert({ clone(instance).into<Asset>() });
	return instance;
}

Arci<TextureAsset> AssetFactory::createTextureAsset(
	mref<asset_guid> guid_,
	cref<asset_guid> baseImage_,
	mref<Vector<asset_guid>> images_,
	cref<math::uivec3> extent_,
	cref<gfx::TextureFormat> format_,
	cref<u32> mipLevel_,
	cref<gfx::TextureType> type_
) const {
	auto instance = Arci<TextureAsset>::create(
		std::move(guid_),
		baseImage_,
		std::forward<Vector<asset_guid>>(images_),
		extent_,
		format_,
		mipLevel_,
		type_
	);

	storeDefaultNameAndUrl(*instance, {});
	const auto result = _registry->insert({ clone(instance).into<Asset>() });

	if (!result) {
		#ifdef _DEBUG
        // TODO: We need to handle a missed insertion call to the asset database
        // Important: Move from asset factory to another place, cause factory is just instantiation, and not database management
		::hg::breakpoint();
		#else
		throw std::runtime_error("Failed to insert texture asset into database.");
		#endif
	}

	return instance;
}
