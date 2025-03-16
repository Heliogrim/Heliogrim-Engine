#pragma once

#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>
#include <Heliogrim/Actor/Actor.hpp>

namespace hg::editor::ui::service {
	enum class SceneHierarchyEntryType {
		eInvalid,
		eActor,
		eComponent
	};

	struct SceneHierarchyEntry {
		using value_type = Variant<nmpt<Actor>, nmpt<HierarchyComponent>>;

		[[nodiscard]] constexpr SceneHierarchyEntryType type() const noexcept {
			switch (value.index()) {
				case 0: return SceneHierarchyEntryType::eActor;
				case 1: return SceneHierarchyEntryType::eComponent;
				default: return SceneHierarchyEntryType::eInvalid;
			}
		}

		value_type value;
	};
}

namespace std {
	template <>
	struct equal_to<::hg::editor::ui::service::SceneHierarchyEntry> {
		[[nodiscard]] constexpr bool operator()(
			::hg::ref<const ::hg::editor::ui::service::SceneHierarchyEntry> left_,
			::hg::ref<const ::hg::editor::ui::service::SceneHierarchyEntry> right_
		) const noexcept {
			return left_.value == right_.value;
		}
	};

	template <>
	struct hash<::hg::editor::ui::service::SceneHierarchyEntry> :
		public std::hash<std::uintptr_t> {
		[[nodiscard]] constexpr size_t operator()(::hg::ref<const ::hg::editor::ui::service::SceneHierarchyEntry> obj_) const noexcept {
			if (obj_.value.is<::hg::nmpt<::hg::Actor>>())
				return static_cast<::hg::ref<const std::hash<std::uintptr_t>>>(*this)(
					reinterpret_cast<std::uintptr_t>(obj_.value.as<::hg::nmpt<::hg::Actor>>().get())
				);
			if (obj_.value.is<::hg::nmpt<::hg::HierarchyComponent>>())
				return static_cast<::hg::ref<const std::hash<std::uintptr_t>>>(*this)(
					reinterpret_cast<std::uintptr_t>(obj_.value.as<::hg::nmpt<::hg::HierarchyComponent>>().get())
				);
			return 0;
		}
	};
}
