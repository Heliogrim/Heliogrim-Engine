#pragma once

#include <span>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "AssetBrowserFilterEntry.hpp"

namespace hg::editor::ui::service {
	struct AssetBrowserFilter {
		ref<const fs::Url> url;
		std::span<const AssetBrowserFilterEntry> typeAndTag;
	};
}