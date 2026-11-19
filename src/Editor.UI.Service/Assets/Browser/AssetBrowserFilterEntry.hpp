#pragma once

#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	using AssetTag = String;

	struct AssetBrowserFilterEntry {
		String name;
		Opt<AssetTag> tag;
		Vector<AssetTypeId> types;
	};
}

namespace std {
	template <>
	struct equal_to<::hg::editor::ui::service::AssetBrowserFilterEntry> {
		[[nodiscard]] constexpr bool operator()(
			::hg::ref<const ::hg::editor::ui::service::AssetBrowserFilterEntry> left_,
			::hg::ref<const ::hg::editor::ui::service::AssetBrowserFilterEntry> right_
		) const noexcept {
			return left_.name == right_.name && left_.types == right_.types;
		}
	};

	template <>
	struct hash<::hg::editor::ui::service::AssetBrowserFilterEntry> {
		[[nodiscard]] constexpr size_t operator()(::hg::ref<const ::hg::editor::ui::service::AssetBrowserFilterEntry> obj_) const noexcept {
			auto tmp = ::hg::hash::fnv1a(obj_.name);
			for (const auto& type : obj_.types) {
				::hg::hash::hashCombine(tmp, ::std::hash<hg::AssetTypeId> {}(type));
			}
			return tmp;
		}
	};
}
