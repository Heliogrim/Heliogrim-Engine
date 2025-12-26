#include "AssetBrowserModel.hpp"

#include <ranges>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserEntry.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserFilter.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserFilterEntry.hpp>
#include <Editor.UI.Service/Assets/Browser/AssetBrowserService.hpp>
#include <Engine.Assets.Type/Geometry/LandscapeGeometry.hpp>
#include <Engine.Assets.Type/Geometry/SkeletalGeometry.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>

#include "AssetBrowserView.hpp"

using namespace hg::editor::ui;
using namespace hg;

/**/

static const auto defaultFilterData = Array {
	service::AssetBrowserFilterEntry { "All", None, {} },
	service::AssetBrowserFilterEntry {
		"Geometry", None,
		{ engine::assets::LandscapeGeometry::typeId, engine::assets::SkeletalGeometry::typeId, engine::assets::StaticGeometry::typeId }
	},
	service::AssetBrowserFilterEntry { "Image", None, { engine::assets::ImageAsset::typeId } },
	service::AssetBrowserFilterEntry { "Material", None, { engine::assets::GfxMaterial::typeId } },
	service::AssetBrowserFilterEntry { "Texture", None, { engine::assets::TextureAsset::typeId } }
};

/**/

AssetBrowserModel::AssetBrowserModel(
	ref<AssetBrowserView> view_,
	SharedPtr<service::AssetBrowserService> service_,
	fs::Path basePath_,
	fs::Path currentPath_
) :
	_view(view_),
	service(::hg::move(service_)),
	basePath(::hg::move(basePath_)),
	currentPath(::hg::move(currentPath_)),
	baseFilterData { std::from_range, defaultFilterData },
	currentFilters {} {}

AssetBrowserModel::~AssetBrowserModel() = default;

void AssetBrowserModel::changeFilters(Opt<std::span<const service::AssetBrowserFilterEntry>> nextFilters_) {

	const auto empty = nextFilters_.map_or(
		[](const auto& filters_) {
			return std::ranges::all_of(
				filters_,
				[](const auto& filterEntry_) {
					return filterEntry_.tag == None && filterEntry_.types.empty();
				}
			);
		},
		true
	);

	if (empty) {
		currentFilters = {};
		return changeDirectory(currentPath);
	}

	/**/

	currentFilters = { std::from_range, nextFilters_.value() };
	return changeDirectory(currentPath);
}

void AssetBrowserModel::changeDirectory(ref<const fs::Path> nextPath_) {

	auto entries = Vector<service::AssetBrowserEntry> {};
	service->fetchAll(
		{
			.url = fs::Url { StringView {}, nextPath_ },
			.typeAndTag = std::span { currentFilters }
		},
		entries
	);

	_view.displayItems(entries);

	/**/

	_view.displayFilterItems(
		baseFilterData,
		[](ref<const service::AssetBrowserFilterEntry> parent_, ref<Vector<service::AssetBrowserFilterEntry>> children_) {
			if (parent_.tag != None)
				return;
			children_.emplace_back("Tag", Some(service::AssetTag { "Tag" }), clone(parent_.types));
		}
	);

	/**/

	auto proxyUrl = std::filesystem::path { nextPath_ };
	Vector<string> proxyParts {};

	while (not proxyUrl
	.
	empty() && proxyUrl != proxyUrl.parent_path()
	) {
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
	return make_uptr < AssetBrowserModel > (
		view_,
		::hg::move(service_),
		::hg::move(basePath_),
		::hg::move(currentPath_)
	);
}