#include "AssetInit.hpp"

#include <Engine.Session/Session.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Ember/Inbuilt.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Assets/AssetTypeId.hpp>

#include <filesystem>
#include <fstream>

#include "Engine.Assets/Database/AssetDatabase.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

using path = _STD filesystem::path;

void dispatchLoad(cref<path> path_) {
    execute([file = path_]() {
        // TODO:

        constexpr auto minLength = sizeof(asset_guid) + sizeof(asset_type_id);

        _STD ifstream ifs { file, _STD ios::in | _STD ios::binary };
        ifs.seekg(0, _STD ios::beg);

        serialization::BufferArchive archive {};
        archive.resize(minLength, 0ui8);

        auto start { ifs.tellg() };
        ifs.read(reinterpret_cast<char*>(archive.data()), archive.size());
        auto end { ifs.tellg() };

        if (ifs.bad()) {
            // Can't even read asset guid or type id, so no viable data
            return;
        }

        auto length { end - start };
        if (length < minLength) {
            // Can't even read asset guid or type id, so no viable data
            return;
        }

        /**/

        asset_guid possibleGuid { invalid_asset_guid };
        asset_type_id possibleTypeId {};

        // TODO: Use predefined data layouts to deserialize guid and type id, cause they are not representable as u64 at every platform
        archive >> reinterpret_cast<ref<u64>>(possibleGuid);
        archive >> reinterpret_cast<ref<u64>>(possibleTypeId);

        if (possibleGuid == invalid_asset_guid) {
            return;
        }

        if (possibleTypeId.data == 0) {
            return;
        }

        /**/

        const auto& layouts = serialization::LayoutManager::get();
        const auto layout = layouts.getLayout(possibleTypeId);

        if (not layout || not layout->hasClass()) {
            // Seams not to be a valid data package, asset type was not bootstrapped correctly or asset is not generically reconstructable
            return;
        }

        /**/

        ifs.seekg(0, _STD ios::end);
        end = ifs.tellg();

        ifs.seekg(0, _STD ios::beg);
        start = ifs.tellg();

        archive.seek(0);
        archive.resize(end - start, 0ui8);

        ifs.read(reinterpret_cast<ptr<char>>(archive.data()), archive.size());

        if (ifs.bad()) {
            // Something went wrong while reading the actual data
            IM_CORE_ERRORF("Received an error while reading possible asset file `{}`", file.string());
            return;
        }

        /**/

        // Warning: Unsafe operation, should be capsulated within io system
        auto* obj { layout->reflect().instantiate() };
        const _STD span<u8, _STD dynamic_extent> view {
            reinterpret_cast<ptr<u8>>(obj),
            static_cast<u64>(layout->size())
        };

        layout->dispatch().load(archive, view);

        /**/

        auto* asset { static_cast<ptr<engine::assets::Asset>>(obj) };
        engine::Session::get()->modules().assetDatabase()->insert(asset->get_guid(), asset->getTypeId(), asset);
    });
}

void tryDispatchLoad(cref<path> path_) {

    if (not path_.extension().string().ends_with(".imasset")) {
        return;
    }

    dispatchLoad(path_);
}

void indexLoadable(cref<path> path_, ref<Vector<path>> backlog_) {

    const auto directory {
        _STD filesystem::directory_iterator { path_, _STD filesystem::directory_options::follow_directory_symlink }
    };

    for (const auto& entry : directory) {

        if (entry.is_directory()) {
            backlog_.push_back(entry.path());
            continue;
        }

        if (entry.is_regular_file()) {
            tryDispatchLoad(entry.path());
            //continue;
        }
    }
}

void editor::boot::initAssets() {

    const auto session { Session::get() };

    /**/

    const auto root { _STD filesystem::current_path().append(R"(..\..\resources\assets)") };
    Vector<path> backlog {};

    backlog.push_back(root);
    while (not backlog.empty()) {

        const auto cur { backlog.back() };
        backlog.pop_back();

        indexLoadable(cur, backlog);
    }

}
