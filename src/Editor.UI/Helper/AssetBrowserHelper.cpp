#include "AssetBrowserHelper.hpp"

#include <Engine.Core/Engine.hpp>

#include <Heliogrim.Default/Assets/Textures/Brand.hpp>
#include <Heliogrim.Default/Assets/Textures/Directory.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderAudio.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderEnvironment.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderFont.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderImages.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderImport.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderLog.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderResource.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderShader.hpp>
#include <Heliogrim.Default/Assets/Textures/FolderVideo.hpp>

#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"

using namespace hg::editor::ui;
using namespace hg;

uptr<AssetBrowserHelper> AssetBrowserHelper::_instance = nullptr;

const ptr<AssetBrowserHelper::this_type> AssetBrowserHelper::get() {
    return _instance.get();
}

const ptr<AssetBrowserHelper::this_type> AssetBrowserHelper::make() {
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

    auto* asset = _assetRegistry->findAssetByGuid(game::assets::texture::Brand::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::Brand());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::Brand::unstable_auto_guid());
    }

    _defaultTypeIcon = static_cast<ptr<engine::assets::Texture>>(asset);

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::Directory::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::Directory());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::Directory::unstable_auto_guid());
    }

    _directoryIcon = static_cast<ptr<engine::assets::Texture>>(asset);

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderAudio::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderAudio());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderAudio::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("audio", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("mp3", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("ogg", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("wav", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImages::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderImages());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImages::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("image", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("ktx", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("png", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("texture", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderResource::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderResource());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderResource::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("resource", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImport::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderImport());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderImport::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("import", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderShader::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderShader());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderShader::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("shader", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderLog::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderLog());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderLog::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("log", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderEnvironment::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderEnvironment());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderEnvironment::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("env", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("environment", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("scene", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("world", static_cast<ptr<engine::assets::Texture>>(asset)));

    _directoryIcons.push_back(_STD make_pair("fbx", static_cast<ptr<engine::assets::Texture>>(asset)));
    _directoryIcons.push_back(_STD make_pair("gltf", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderVideo::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderVideo());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderVideo::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("video", static_cast<ptr<engine::assets::Texture>>(asset)));

    /**/

    asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderFont::unstable_auto_guid());
    if (asset == nullptr) {
        delete (new game::assets::texture::FolderFont());
        asset = _assetRegistry->findAssetByGuid(game::assets::texture::FolderFont::unstable_auto_guid());
    }

    _directoryIcons.push_back(_STD make_pair("font", static_cast<ptr<engine::assets::Texture>>(asset)));
}

ptr<engine::assets::Texture> AssetBrowserHelper::getItemIconByAssetType(cref<asset_type_id> typeId_) const noexcept {

    const auto it { _typeIconMap.find(typeId_) };
    if (it != _typeIconMap.end()) {
        return it->second;
    }
    return _defaultTypeIcon;
}

ptr<engine::assets::Texture> AssetBrowserHelper::getItemIconForDirectory(cref<string_view> name_) const noexcept {

    s32 length { static_cast<s32>(name_.size()) - 2 };
    length = MAX(length, 0);

    ptr<engine::assets::Texture> tex { _directoryIcon };

    string name { name_ };
    _STD transform(
        name.begin(),
        name.end(),
        name.begin(),
        [](const u8 c) {
            return _STD tolower(c);
        }
    );

    for (const auto& entry : _directoryIcons) {
        if (name.contains(entry.first) && entry.first.size() > length) {

            length = entry.first.size();
            tex = entry.second;

            if (length == name.size()) {
                break;
            }
        }
    }

    return tex;
}

#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.Assets/Types/Font.hpp>

string AssetBrowserHelper::getAssetTypeName(cref<asset_type_id> typeId_) const noexcept {

    static constexpr asset_type_id invalid {};

    switch (typeId_.data) {
        case engine::assets::Image::typeId.data: {
            return "Image";
        }
        case engine::assets::Texture::typeId.data: {
            return "Texture";
        }
        case engine::assets::StaticGeometry::typeId.data: {
            return "Static Geometry";
        }
        case engine::assets::GfxMaterial::typeId.data: {
            return "Gfx Material";
        }
        case engine::assets::Font::typeId.data: {
            return "Font";
        }
        case invalid.data: {
            return "Undefined";
        }
        default: {
            return _STD to_string(typeId_.data);
        }
    }
}
