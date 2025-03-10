#include "AssetBrowserController.hpp"

#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>

#include "AssetBrowserModel.hpp"
#include "AssetBrowserView.hpp"

using namespace hg::editor::ui;
using namespace hg;

AssetBrowserController::AssetBrowserController() :
	_model(),
	_view() {}

AssetBrowserController::~AssetBrowserController() = default;

void AssetBrowserController::setup(mref<AssetBrowserOptions> options_) {
	_view = makeAssetBrowserView(*this);

	const auto initPath = options_.initialPath.has_value() ? ::hg::move(options_.initialPath).value() : clone(options_.basePath);
	_model = makeAssetBrowserModel(*_view, ::hg::move(options_.service), ::hg::move(options_.basePath), clone(initPath));

	/**/

	_model->changeDirectory(initPath);
}

ref<const AssetBrowserModel> AssetBrowserController::getModel() const noexcept {
	return *_model;
}

ref<const AssetBrowserView> AssetBrowserController::getView() const noexcept {
	return *_view;
}

void AssetBrowserController::onNavItemClick(ref<const fs::Url> eventData_) {
	_model->changeDirectory(eventData_);
}

void AssetBrowserController::onAssetItemClick(ref<const service::AssetBrowserEntry> eventData_) {
	_model->changeDirectory(eventData_.path);
}

UniquePtr<AssetBrowserController> editor::ui::makeAssetBrowser(AssetBrowserOptions options_) {
	auto controller = make_uptr<AssetBrowserController>();
	controller->setup(::hg::move(options_));
	return controller;
}
