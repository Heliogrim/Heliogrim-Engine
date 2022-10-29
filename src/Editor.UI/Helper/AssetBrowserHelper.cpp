#include "AssetBrowserHelper.hpp"

#include <Engine.Session/Session.hpp>

#include <Ember.Default/Assets/Textures/Brand.hpp>
#include <Ember.Default/Assets/Textures/Directory.hpp>
#include <Ember.Default/Assets/Textures/FolderAudio.hpp>
#include <Ember.Default/Assets/Textures/FolderImages.hpp>
#include <Ember.Default/Assets/Textures/FolderLog.hpp>
#include <Ember.Default/Assets/Textures/FolderResource.hpp>
#include <Ember.Default/Assets/Textures/FolderShader.hpp>

using namespace ember::editor::ui;
using namespace ember;

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

    const auto& session { engine::Session::get() };
    _assetDb = session->modules().assetDatabase();

    /**/

    auto query { _assetDb->query(game::assets::texture::Brand::auto_guid()) };
    if (not query.exists()) {
        delete (new game::assets::texture::Brand());
    }

    _defaultTypeIcon = static_cast<ptr<engine::assets::Texture>>(query.get());

    /**/

    query = _assetDb->query(game::assets::texture::Directory::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::Directory());
    }

    _directoryIcon = static_cast<ptr<engine::assets::Texture>>(query.get());

    /**/

    query = _assetDb->query(game::assets::texture::FolderAudio::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderAudio());
    }

    _directoryIcons.push_back(_STD make_pair("audio", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("mp3", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("ogg", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("wav", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderImages::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderImages());
    }

    _directoryIcons.push_back(_STD make_pair("image", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("ktx", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("png", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("texture", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderResource::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderResource());
    }

    _directoryIcons.push_back(_STD make_pair("import", static_cast<ptr<engine::assets::Texture>>(query.get())));
    _directoryIcons.push_back(_STD make_pair("resource", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderShader::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderShader());
    }

    _directoryIcons.push_back(_STD make_pair("shader", static_cast<ptr<engine::assets::Texture>>(query.get())));

    /**/

    query = _assetDb->query(game::assets::texture::FolderLog::auto_guid());
    if (not query.exists()) {
        delete (new game::assets::texture::FolderLog());
    }

    _directoryIcons.push_back(_STD make_pair("log", static_cast<ptr<engine::assets::Texture>>(query.get())));
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
    _STD transform(name.begin(), name.end(), name.begin(), [](const u8 c) {
        return _STD tolower(c);
    });

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
