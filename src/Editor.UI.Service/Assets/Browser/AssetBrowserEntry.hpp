#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets;
#endif

namespace hg::editor::ui::service {
	enum class AssetBrowserEntryType {
		eUndefined,
		//
		eDirectory,
		eFile,
		//
		eAsset
	};

	/**/

	struct AssetBrowserEntry {
		AssetBrowserEntryType type;
		engine::assets::AssetUrl url;
		AssetGuid guid;
	};
}
