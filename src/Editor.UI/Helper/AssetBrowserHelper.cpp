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
    _assetDb(nullptr),
    _defaultTypeIcon(nullptr),
    _typeIconMap(),
    _directoryIcon(nullptr) {
    setup();
}

AssetBrowserHelper::~AssetBrowserHelper() noexcept = default;

void AssetBrowserHelper::setup() {

    _assetDb = engine::Engine::getEngine()->getAssets()->getDatabase();

    /**/

    auto query { _assetDb->query(game::assets::texture::Brand::unstable_auto_guid()) };
    if (not query.exists()) {
        delete (new game::assets::texture::Brand());
    }

    _defaultTypeIcon = static_cast<ptr<engine::assets::Texture>>(query.get());

    /**/

    query = _assetDb->query(game::assets::texture::Directory::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::Directory());
    }

    _directoryIcon = static_cast<ptr<engine::assets::Texture>>(query.get());

    /**/

    query = _assetDb->query(game::assets::texture::FolderAudio::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderAudio());
    }

    _directoryIcons.push_back(_STD make_pair("audio", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("mp3", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("ogg", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("wav", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderImages::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderImages());
    }

    _directoryIcons.push_back(_STD make_pair("image", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("ktx", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("png", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("texture", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderResource::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderResource());
    }

    _directoryIcons.push_back(_STD make_pair("resource", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderImport::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderImport());
    }

    _directoryIcons.push_back(_STD make_pair("import", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderShader::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderShader());
    }

    _directoryIcons.push_back(_STD make_pair("shader", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderLog::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderLog());
    }

    _directoryIcons.push_back(_STD make_pair("log", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderEnvironment::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderEnvironment());
    }

    _directoryIcons.push_back(_STD make_pair("env", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("environment", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("scene", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("world", static_cast<ptr<engine::assets::Texture>>(query.get())));

    _directoryIcons.push_back(_STD make_pair("fbx", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("gltf", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderVideo::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderVideo());
    }

    _directoryIcons.push_back(_STD make_pair("video", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderFont::unstable_auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderFont());
    }

    _directoryIcons.push_back(_STD make_pair("font", static_cast<ptr<engine::assets::Texture>>(query.get())));
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

asset_type_id AssetBrowserHelper::getAssetTypeByFile(cref<Url> fqUrl_) const noexcept {
    return {};
}

string AssetBrowserHelper::getAssetTypeName(cref<asset_type_id> typeId_) const noexcept {
    return _STD to_string(typeId_.data);
}
