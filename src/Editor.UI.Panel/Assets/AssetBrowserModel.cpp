#include "AssetBrowserModel.hpp"

#include <Editor.UI.Service/Assets/Browser/AssetBrowserService.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>

#include "AssetBrowserView.hpp"

using namespace hg::editor::ui;
using namespace hg;

AssetBrowserModel::AssetBrowserModel(
	ref<AssetBrowserView> view_,
	SharedPtr<service::AssetBrowserService> service_,
	fs::Path basePath_,
	fs::Path currentPath_
) :
	_view(view_),
	service(::hg::move(service_)),
	basePath(::hg::move(basePath_)),
	currentPath(::hg::move(currentPath_)) {}

AssetBrowserModel::~AssetBrowserModel() = default;

void AssetBrowserModel::changeDirectory(ref<const fs::Path> nextPath_) {

	auto entries = Vector<service::AssetBrowserEntry> {};
	service->fetchAll(fs::Url { StringView {}, nextPath_ }, entries);

	_view.displayItems(entries);

	/**/

	auto proxyUrl = std::filesystem::path { nextPath_ };
	Vector<string> proxyParts {};

	while (not proxyUrl.empty() && proxyUrl != proxyUrl.parent_path()) {
		proxyParts.emplace_back(proxyUrl.filename().string());
		proxyUrl = proxyUrl.parent_path();
	}

	if (proxyUrl.has_root_name() && proxyUrl.root_name().native().size() != basePath.name().size()) {
		proxyParts.push_back(
			proxyUrl.root_name().string().substr(
				static_cast<String>(basePath).size() + 1/* Separator Size */
			)
		);
	}

	proxyParts.emplace_back(basePath);

	/**/

	auto navItemData = Vector<BrowserNavEntry> {};

	auto fwd = fs::Path { ""sv };
	for (auto it { proxyParts.rbegin() }; it != proxyParts.rend(); ++it) {

		const auto& part { *it };
		if (part == static_cast<String>(basePath)) {
			fwd = basePath;
		} else if (fwd == basePath) {
			fwd = fs::Path { part };
		} else {
			fwd = fs::Path { fwd }.push(fs::Path { part });
		}

		const string title { (part == static_cast<String>(basePath)) ? "Root" : part };

		navItemData.emplace_back(title, fwd);
	}

	/**/

	_view.displayNav(navItemData);

	currentPath = nextPath_;
}

UniquePtr<AssetBrowserModel> editor::ui::makeAssetBrowserModel(
	ref<AssetBrowserView> view_,
	SharedPtr<service::AssetBrowserService> service_,
	fs::Path basePath_,
	fs::Path currentPath_
) {
	return make_uptr<AssetBrowserModel>(
		view_,
		::hg::move(service_),
		::hg::move(basePath_),
		::hg::move(currentPath_)
	);
}
