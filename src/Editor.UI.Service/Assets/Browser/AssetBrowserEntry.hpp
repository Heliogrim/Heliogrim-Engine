#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetUrl.hpp>

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
