#include "AssetInit.hpp"

#include <Engine.Session/Session.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Ember/Inbuilt.hpp>

#include <filesystem>

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

using path = _STD filesystem::path;

void dispatchLoad(cref<path> path_) {
    execute([file = path_]() {
        // TODO:
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
    while (not root.empty()) {

        const auto cur { backlog.back() };
        backlog.pop_back();

        indexLoadable(cur, backlog);
    }

}
