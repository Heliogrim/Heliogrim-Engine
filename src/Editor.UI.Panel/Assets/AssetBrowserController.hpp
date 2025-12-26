#pragma once

#include <span>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Filesystem/Url.hpp>

namespace hg::editor::ui {
	class AssetBrowserModel;
	class AssetBrowserView;
}

namespace hg::editor::ui::service {
	class AssetBrowserService;
	struct AssetBrowserEntry;
	struct AssetBrowserFilterEntry;
}

namespace hg::editor::ui {
	struct AssetBrowserOptions {
		SharedPtr<service::AssetBrowserService> service;
		fs::Path basePath;
		Opt<fs::Path> initialPath;
	};

	/**/

	class AssetBrowserController {
	public:
		AssetBrowserController();

		~AssetBrowserController();

	private:
		UniquePtr<AssetBrowserModel> _model;
		UniquePtr<AssetBrowserView> _view;

	public:
		void setup(mref<AssetBrowserOptions> options_);

	public:
		[[nodiscard]] ref<const AssetBrowserModel> getModel() const noexcept;

		[[nodiscard]] ref<const AssetBrowserView> getView() const noexcept;

	public:
		void onFilterItemClick(Opt<std::span<const service::AssetBrowserFilterEntry>> filters_);

		void onNavItemClick(ref<const fs::Path> eventData_);

		void onAssetItemClick(ref<const service::AssetBrowserEntry> eventData_);
	};

	/**/

	[[nodiscard]] extern UniquePtr<AssetBrowserController> makeAssetBrowser(AssetBrowserOptions options_);
}