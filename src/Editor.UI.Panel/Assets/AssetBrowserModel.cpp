#include "AssetBrowserModel.hpp"

#include <Editor.UI.Service/Assets/Browser/AssetBrowserService.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>

#include "AssetBrowserView.hpp"

using namespace hg::editor::ui;
using namespace hg;

AssetBrowserModel::AssetBrowserModel(
	ref<AssetBrowserView> view_,
	SharedPtr<service::AssetBrowserService> service_,
	fs::Url basePath_,
	fs::Url currentPath_
) :
	_view(view_),
	service(::hg::move(service_)),
	basePath(::hg::move(basePath_)),
	currentPath(::hg::move(currentPath_)) {}

AssetBrowserModel::~AssetBrowserModel() = default;

void AssetBrowserModel::changeDirectory(ref<const fs::Url> nextPath_) {

	auto entries = Vector<service::AssetBrowserEntry> {};
	service->fetchAll(nextPath_, entries);

	_view.displayItems(entries);

	/**/

	auto proxyUrl = std::filesystem::path { nextPath_.path() };
	Vector<string> proxyParts {};

	while (not proxyUrl.empty() && proxyUrl != proxyUrl.parent_path()) {
		proxyParts.emplace_back(proxyUrl.filename().string());
		proxyUrl = proxyUrl.parent_path();
	}

	if (proxyUrl.has_root_name() && proxyUrl.root_name().native().size() != basePath.path().name().size()) {
		proxyParts.push_back(
			proxyUrl.root_name().string().substr(
				static_cast<String>(basePath.path()).size() + 1/* Separator Size */
			)
		);
	}

	proxyParts.emplace_back(basePath.path());

	/**/

	auto navItemData = Vector<BrowserNavEntry> {};

	fs::Url fwd { basePath.scheme(), ""sv };
	for (auto it { proxyParts.rbegin() }; it != proxyParts.rend(); ++it) {

		const auto& part { *it };
		if (part == static_cast<String>(basePath.path())) {
			fwd = basePath;
		} else if (fwd.path() == basePath.path()) {
			fwd = fs::Url { fwd.scheme(), std::filesystem::path { part }.string() };
		} else {
			fwd = fs::Url { fwd.scheme(), std::filesystem::path { fwd.path() }.append(part).string() };
		}

		const string title { (part == static_cast<String>(basePath.path())) ? "Root" : part };

		navItemData.emplace_back(title, fwd);
	}

	/**/

	_view.displayNav(navItemData);

	currentPath = nextPath_;
}

UniquePtr<AssetBrowserModel> editor::ui::makeAssetBrowserModel(
	ref<AssetBrowserView> view_,
	SharedPtr<service::AssetBrowserService> service_,
	fs::Url basePath_,
	fs::Url currentPath_
) {
	return make_uptr<AssetBrowserModel>(
		view_,
		::hg::move(service_),
		::hg::move(basePath_),
		::hg::move(currentPath_)
	);
}
