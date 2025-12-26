#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Filesystem/Url.hpp>

namespace hg::editor::ui {
	class AssetBrowserView;
}

namespace hg::editor::ui::service {
	struct AssetBrowserFilterEntry;
	class AssetBrowserService;
}

namespace hg::editor::ui {
	class AssetBrowserModel {
	public:
		AssetBrowserModel(
			ref<AssetBrowserView> view_,
			SharedPtr<service::AssetBrowserService> service_,
			fs::Path basePath_,
			fs::Path currentPath_
		);

		~AssetBrowserModel();

	private:
		ref<AssetBrowserView> _view;

	public:
		SharedPtr<service::AssetBrowserService> service;
		fs::Path basePath;
		fs::Path currentPath;

		Vector<service::AssetBrowserFilterEntry> baseFilterData;
		Vector<service::AssetBrowserFilterEntry> currentFilters;

	public:
		void changeFilters(Opt<std::span<const service::AssetBrowserFilterEntry>> nextFilters_);

		void changeDirectory(ref<const fs::Path> nextPath_);
	};

	[[nodiscard]] extern UniquePtr<AssetBrowserModel> makeAssetBrowserModel(
		_Inout_ ref<AssetBrowserView> view_,
		SharedPtr<service::AssetBrowserService> service_,
		fs::Path basePath_,
		fs::Path currentPath_
	);
}