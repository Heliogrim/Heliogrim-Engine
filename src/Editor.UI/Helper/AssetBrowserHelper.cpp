#include "AssetBrowserHelper.hpp"

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Editor.Assets.Default/Textures/Brand.hpp>
#include <Editor.Assets.Default/Textures/Directory.hpp>
#include <Editor.Assets.Default/Textures/FolderAudio.hpp>
#include <Editor.Assets.Default/Textures/FolderEnvironment.hpp>
#include <Editor.Assets.Default/Textures/FolderFont.hpp>
#include <Editor.Assets.Default/Textures/FolderImages.hpp>
#include <Editor.Assets.Default/Textures/FolderImport.hpp>
#include <Editor.Assets.Default/Textures/FolderLog.hpp>
#include <Editor.Assets.Default/Textures/FolderResource.hpp>
#include <Editor.Assets.Default/Textures/FolderShader.hpp>
#include <Editor.Assets.Default/Textures/FolderVideo.hpp>

using namespace hg::editor::ui;
using namespace hg;

uptr<AssetBrowserHelper> AssetBrowserHelper::_instance = nullptr;

nmpt<AssetBrowserHelper::this_type> AssetBrowserHelper::get() {
	return _instance.get();
}

nmpt<AssetBrowserHelper::this_type> AssetBrowserHelper::make() {
	if (!_instance) {
		_instance = uptr<AssetBrowserHelper>(new AssetBrowserHelper());
	}
	return _instance.get();
}

void AssetBrowserHelper::destroy() {
	_instance.reset();
}

AssetBrowserHelper::AssetBrowserHelper() :
	_assetRegistry(nullptr),
	_defaultTypeIcon(nullptr),
	_typeIconMap(),
	_directoryIcon(nullptr) {
	setup();
}

AssetBrowserHelper::~AssetBrowserHelper() noexcept = default;

void AssetBrowserHelper::setup() {

	_assetRegistry = engine::Engine::getEngine()->getAssets()->getRegistry();

	/**/

	auto asset = _assetRegistry->findAssetByGuid(game::assets::texture::Brand::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::Brand());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::Brand::unstable_auto_guid());
	}

	_defaultTypeIcon = Cast<engine::assets::TextureAsset>(asset.get());

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::Directory::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::Directory());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::Directory::unstable_auto_guid());
	}

	_directoryIcon = Cast<engine::assets::TextureAsset>(asset.get());

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderAudio::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderAudio());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderAudio::unstable_auto_guid());
	}

	_directoryIcons.emplace_back("audio", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("mp3", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("ogg", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("wav", Cast<engine::assets::TextureAsset>(asset.get()));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImages::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderImages());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImages::unstable_auto_guid());
	}

	_directoryIcons.emplace_back("image", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("ktx", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("png", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("texture", Cast<engine::assets::TextureAsset>(asset.get()));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderResource::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderResource());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderResource::unstable_auto_guid());
	}

	_directoryIcons.push_back(std::make_pair("resource", Cast<engine::assets::TextureAsset>(asset.get())));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImport::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderImport());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImport::unstable_auto_guid());
	}

	_directoryIcons.push_back(std::make_pair("import", Cast<engine::assets::TextureAsset>(asset.get())));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderShader::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderShader());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderShader::unstable_auto_guid());
	}

	_directoryIcons.push_back(std::make_pair("shader", Cast<engine::assets::TextureAsset>(asset.get())));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderLog::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderLog());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderLog::unstable_auto_guid());
	}

	_directoryIcons.push_back(std::make_pair("log", Cast<engine::assets::TextureAsset>(asset.get())));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderEnvironment::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderEnvironment());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderEnvironment::unstable_auto_guid());
	}

	_directoryIcons.emplace_back("env", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("environment", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("scene", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("world", Cast<engine::assets::TextureAsset>(asset.get()));

	_directoryIcons.emplace_back("fbx", Cast<engine::assets::TextureAsset>(asset.get()));
	_directoryIcons.emplace_back("gltf", Cast<engine::assets::TextureAsset>(asset.get()));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderVideo::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderVideo());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderVideo::unstable_auto_guid());
	}

	_directoryIcons.emplace_back("video", Cast<engine::assets::TextureAsset>(asset.get()));

	/**/

	asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderFont::unstable_auto_guid());
	if (asset == nullptr) {
		delete (new game::assets::texture::FolderFont());
		asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderFont::unstable_auto_guid());
	}

	_directoryIcons.emplace_back("font", Cast<engine::assets::TextureAsset>(asset.get()));
}

ptr<engine::assets::TextureAsset>
AssetBrowserHelper::getItemIconByAssetType(cref<asset_type_id> typeId_) const noexcept {

	const auto it { _typeIconMap.find(typeId_) };
	if (it != _typeIconMap.end()) {
		return it->second;
	}
	return _defaultTypeIcon;
}

ptr<engine::assets::TextureAsset> AssetBrowserHelper::getItemIconForDirectory(cref<string_view> name_) const noexcept {

	s32 length { static_cast<s32>(name_.size()) - 2 };
	length = MAX(length, 0);

	ptr<engine::assets::TextureAsset> tex { _directoryIcon };

	string name { name_ };
	std::transform(
		name.begin(),
		name.end(),
		name.begin(),
		[](const u8 c) {
			return std::tolower(c);
		}
	);

	for (const auto& entry : _directoryIcons) {
		if (name.contains(entry.first) && entry.first.size() > length) {

			length = static_cast<s32>(entry.first.size());
			tex = entry.second;

			if (length == name.size()) {
				break;
			}
		}
	}

	return tex;
}

#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Assets/Types/Material/GfxMaterial.hpp>
#include <Engine.Assets/Types/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>

string AssetBrowserHelper::getAssetTypeName(cref<asset_type_id> typeId_) const noexcept {

	static constexpr asset_type_id invalid {};

	switch (typeId_.data) {
		case engine::assets::Image::typeId.data: {
			return "Image";
		}
		case engine::assets::TextureAsset::typeId.data: {
			return "Texture";
		}
		case engine::assets::StaticGeometry::typeId.data: {
			return "Static Geometry";
		}
		case engine::assets::GfxMaterial::typeId.data: {
			return "Gfx Material";
		}
		case engine::assets::GfxMaterialPrototype::typeId.data: {
			return "Gfx Material Prototype";
		}
		case engine::assets::Font::typeId.data: {
			return "Font";
		}
		case invalid.data: {
			return "Undefined";
		}
		default: {
			return std::to_string(typeId_.data);
		}
	}
}
