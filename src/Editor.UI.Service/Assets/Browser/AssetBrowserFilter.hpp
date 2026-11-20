#pragma once

#include <Engine.Filesystem/Url.hpp>

#include "AssetBrowserFilterEntry.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	struct AssetBrowserFilter {
		ref<const fs::Url> url;
		std::span<const AssetBrowserFilterEntry> typeAndTag;
	};
}
