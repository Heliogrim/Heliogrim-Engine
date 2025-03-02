#pragma once

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
}

namespace hg::editor::ui {
	struct AssetBrowserOptions {
		SharedPtr<service::AssetBrowserService> service;
		fs::Url basePath;
		Opt<fs::Url> initialPath;
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
		void onNavItemClick(ref<const fs::Url> eventData_);

		void onAssetItemClick(ref<const service::AssetBrowserEntry> eventData_);
	};

	/**/

	[[nodiscard]] extern UniquePtr<AssetBrowserController> makeAssetBrowser(AssetBrowserOptions options_);
}
