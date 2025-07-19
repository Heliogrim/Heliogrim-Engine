#include "AssetBrowserHelper.hpp"

#include <Editor.Assets.Default/Textures/Default.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

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

	auto asset = assets::texture::get_default_brand();
	_defaultTypeIcon = Cast<engine::assets::TextureAsset>(asset->get());

	/**/

	asset = assets::texture::get_default_folder();
	_directoryIcon = Cast<engine::assets::TextureAsset>(asset->get());

	/**/

	asset = assets::texture::get_default_folder_audio();
	_directoryIcons.emplace_back("audio", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("mp3", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("ogg", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("wav", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_images();
	_directoryIcons.emplace_back("image", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("ktx", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("png", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("texture", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_resource();
	_directoryIcons.emplace_back("resource", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_import();
	_directoryIcons.emplace_back("import", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_shader();
	_directoryIcons.emplace_back("shader", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_log();
	_directoryIcons.emplace_back("log", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_environment();
	_directoryIcons.emplace_back("env", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("environment", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("scene", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("universe", Cast<engine::assets::TextureAsset>(asset->get()));

	_directoryIcons.emplace_back("fbx", Cast<engine::assets::TextureAsset>(asset->get()));
	_directoryIcons.emplace_back("gltf", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_video();
	_directoryIcons.emplace_back("video", Cast<engine::assets::TextureAsset>(asset->get()));

	/**/

	asset = assets::texture::get_default_folder_font();
	_directoryIcons.emplace_back("font", Cast<engine::assets::TextureAsset>(asset->get()));
}

ptr<engine::assets::TextureAsset> AssetBrowserHelper::getItemIconByAssetType(cref<AssetTypeId> typeId_) const noexcept {

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

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>

string AssetBrowserHelper::getAssetTypeName(cref<AssetTypeId> typeId_) const noexcept {

	static constexpr AssetTypeId invalid {};

	switch (typeId_.data) {
		case engine::assets::ImageAsset::typeId.data: {
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
		case engine::assets::FontAsset::typeId.data: {
			return "FontAsset";
		}
		case invalid.data: {
			return "Undefined";
		}
		default: {
			return std::to_string(typeId_.data);
		}
	}
}
