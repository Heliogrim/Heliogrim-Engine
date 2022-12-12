#include "SimpleImportAction.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/ImporterManager.hpp>

#include "Engine.Assets/Types/Texture/Texture.hpp"
#include "Engine.Assets/Types/Image.hpp"

using namespace ember::editor;
using namespace ember;

SimpleImportAction::SimpleImportAction() :
    ImportAction(typeId),
    _source(),
    _target(),
    _assets(),
    _running(),
    _finished(),
    _failed() {}

SimpleImportAction::SimpleImportAction(cref<Url> source_, cref<Url> target_) :
    ImportAction(typeId),
    _source(source_),
    _target(target_),
    _assets(),
    _running(),
    _finished(),
    _failed() {}

SimpleImportAction::~SimpleImportAction() {}

Url SimpleImportAction::getSourceUrl() const noexcept {
    return _source;
}

Url SimpleImportAction::getTargetUrl() const noexcept {
    return _target;
}

cref<decltype(SimpleImportAction::_assets)> SimpleImportAction::importedAssets() const noexcept {
    return _assets;
}

void SimpleImportAction::setRunning() {
    _finished.clear();
    _running.test_and_set();
}

void SimpleImportAction::setFinished() {
    _running.clear();
    _finished.test_and_set();
}

bool SimpleImportAction::isReversible() const noexcept {
    return true;
}

bool SimpleImportAction::isPending() const noexcept {
    return not _running.test() && not _finished.test();
}

bool SimpleImportAction::isRunning() const noexcept {
    return _running.test();
}

bool SimpleImportAction::isFinished() const noexcept {
    return _finished.test();
}

#include "Engine.GFX/Importer/ImageFileTypes.hpp"

void SimpleImportAction::apply() {
    setRunning();
    _failed = false;

    const auto* const manager { engine::Engine::getEngine()->getResources() };
    const auto& importer { manager->importer() };

    /**/

    const auto srcPath { _source.path() };
    const ::ember::File file { srcPath };

    if (srcPath.ends_with(".ktx2")) {
        const auto result = importer.import<
            _STD pair<ptr<::ember::engine::assets::Texture>, ptr<::ember::engine::assets::Image>>
        >(::ember::engine::gfx::ImageFileType::Ktx2, file);

        const auto data { result.get() };

        if (data.first == nullptr && data.second == nullptr) {
            _failed = true;
        }

        if (data.first) {
            _assets.insert(data.first);
        }

        if (data.second) {
            _assets.insert(data.second);
        }
    }

    setFinished();
}

void SimpleImportAction::reverse() {
    setRunning();
    _failed = false;
    setFinished();
}

SimpleImportAction::operator ptr<await_signal_sub_type>() const noexcept {
    return &_finished;
}

bool SimpleImportAction::failed() const noexcept {
    return _failed;
}
