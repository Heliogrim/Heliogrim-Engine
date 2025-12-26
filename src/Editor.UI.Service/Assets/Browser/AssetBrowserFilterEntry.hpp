#pragma once

#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>
#include <Engine.Common/Math/Hash.hpp>

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
