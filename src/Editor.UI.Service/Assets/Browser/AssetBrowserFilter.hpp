#pragma once

#include "AssetBrowserFilterEntry.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Filesystem;
#endif

namespace hg::editor::ui::service {
	struct AssetBrowserFilter {
		ref<const fs::Url> url;
		std::span<const AssetBrowserFilterEntry> typeAndTag;
	};
}
