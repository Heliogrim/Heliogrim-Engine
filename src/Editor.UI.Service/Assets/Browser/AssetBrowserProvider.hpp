#pragma once

#include <Engine.Common/Sal.hpp>

#include "AssetBrowserEntry.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Filesystem;
#endif

namespace hg::editor::ui::service {
	class AssetBrowserProvider {
	public:
		using this_type = AssetBrowserProvider;

	public:
		constexpr virtual ~AssetBrowserProvider() noexcept = default;

	public:
		[[nodiscard]] virtual bool effects(ref<const AssetBrowserFilter> filter_) const = 0;

		virtual bool fetchItems(ref<const AssetBrowserFilter> filter_, _Inout_ ref<Vector<AssetBrowserEntry>> entries_) const = 0;

		virtual bool fetchDirectories(ref<const AssetBrowserFilter> filter_, _Inout_ ref<Vector<AssetBrowserEntry>> directories_) const = 0;
	};
}
