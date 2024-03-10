#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "AssetBrowserEntry.hpp"

namespace hg::editor::ui {
	class AssetBrowserProvider {
	public:
		using this_type = AssetBrowserProvider;

	public:
		virtual ~AssetBrowserProvider() = default;

	public:
		virtual bool effects(cref<fs::Url> url_) = 0;

		virtual bool retrieve(cref<fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> entries_) = 0;

		virtual bool retrieveDirectories(cref<fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> directories_) = 0;
	};
}
