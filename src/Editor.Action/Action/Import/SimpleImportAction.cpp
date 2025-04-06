#include "SimpleImportAction.hpp"

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Resource/File.hpp>
#include <Engine.Resource/ImporterManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>

using namespace hg::editor;
using namespace hg;

SimpleImportAction::SimpleImportAction() :
	InheritMeta(),
	_source(),
	_target(),
	_assets(),
	_running(),
	_finished() {}

SimpleImportAction::SimpleImportAction(cref<fs::Url> source_, cref<fs::Url> target_) :
	InheritMeta(),
	_source(source_),
	_target(target_),
	_assets(),
	_running(),
	_finished() {}

SimpleImportAction::~SimpleImportAction() = default;

fs::Url SimpleImportAction::getSourceUrl() const noexcept {
	return _source;
}

fs::Url SimpleImportAction::getTargetUrl() const noexcept {
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
#include "Engine.GFX/Importer/ModelFileTypes.hpp"

Result<void, std::runtime_error> SimpleImportAction::apply() {
	setRunning();

	const auto manager { engine::Engine::getEngine()->getResources() };
	const auto& importer { manager->importer() };

	/**/

	const auto srcPath = static_cast<String>(_source.path());
	const ::hg::fs::File file { srcPath };

	if (srcPath.ends_with(".ktx2")) {
		const auto result = importer.import<
			std::pair<Arci<::hg::engine::assets::TextureAsset>, Arci<::hg::engine::assets::Image>>
		>(::hg::engine::gfx::ImageFileType::Ktx2, file);

		auto data { result.get() };

		if (data.first == nullptr && data.second == nullptr) {
			setFinished();
			return Unexpected { std::runtime_error { "Failed to import texture asset from ktx2 file." } };
		}

		if (data.first != nullptr) {
			_assets.insert(std::move(data.first).into<::hg::engine::assets::Asset>());
		}

		if (data.second != nullptr) {
			_assets.insert(std::move(data.second).into<::hg::engine::assets::Asset>());
		}

	} else if (srcPath.ends_with(".fbx")) {

		const auto result = importer.import<
			Arci<engine::assets::StaticGeometry>
		>(::hg::engine::gfx::ModelFileType::Fbx, file);

		auto data = result.get();

		if (data == nullptr) {
			setFinished();
			return Unexpected { std::runtime_error { "Failed to import static geometry from fbx file." } };

		} else {
			_assets.insert(std::move(data).into<::hg::engine::assets::Asset>());
		}
	}

	setFinished();
}

Result<void, std::runtime_error> SimpleImportAction::revoke() {
	setRunning();
	setFinished();
	return Expected<void> {};
}

Result<void, std::runtime_error> SimpleImportAction::undo() {
	setRunning();
	setFinished();
	return Unexpected { std::runtime_error { "TODO: Failed to undo import of ..." } };
}

SimpleImportAction::operator ptr<await_signal_sub_type>() const noexcept {
	return &_finished;
}
