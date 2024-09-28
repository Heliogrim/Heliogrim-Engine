#pragma once

#include <Engine.Core/Module/RootModule.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "FnSets.hpp"
#include "Archive/LayoutArchive.hpp"
#include "Archive/StructuredArchive.hpp"
#include "Structure/StructScopedSlot.hpp"

namespace hg::engine {
	class SerializationModule final :
		public core::RootModule,
		public serialization::FnSets {
	public:
		using this_type = SerializationModule;

	public:
		explicit SerializationModule() = default;

		SerializationModule(cref<this_type>) = delete;

		SerializationModule(mref<this_type>) = delete;

		~SerializationModule() = default;

	public:
		decltype(auto) serialize(_In_ auto&& obj_, _Inout_ mref<serialization::LayoutArchiveBase> archive_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			auto archive = static_cast<mref<serialization::TypedLayoutArchive<target_type>>>(archive_);
			// TODO: ...
			return std::addressof(archive) != nullptr ? Some(std::move(archive)) : None;
		}

		decltype(auto) serialize(_In_ auto&& obj_, _Inout_ ref<serialization::StructuredArchive> archive_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			serialization::access::Structure<target_type>::serialize(
				std::forward<decltype(obj_)>(obj_),
				archive_.insertRootSlot().intoStruct()
			);
			return not archive_.getArchive().hasError() ? Some(archive_) : None;
		}

		decltype(auto) serialize(_In_ auto&& obj_, _Inout_ mref<serialization::StructScopedSlot> slot_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			serialization::access::Structure<target_type>::serialize(std::forward<decltype(obj_)>(obj_), clone(slot_));
			return slot_.slot() ? Some(std::move(slot_)) : None;
		}

	public:
		decltype(auto) hydrate(_In_ cref<serialization::LayoutArchiveBase> archive_, _Inout_ auto& obj_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			auto archive = static_cast<mref<serialization::TypedLayoutArchive<target_type>>>(archive_);
			// TODO: ...
			return std::addressof(obj_) != nullptr ? Some(obj_) : None;
		}

		decltype(auto) hydrate(_In_ cref<serialization::StructuredArchive> archive_, _Inout_ auto& obj_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			serialization::access::Structure<target_type>::hydrate(
				archive_.getRootSlot().intoStruct(),
				obj_
			);
			return std::addressof(obj_) != nullptr ? Some(obj_) : None;
		}

		decltype(auto) hydrate(_In_ cref<serialization::StructScopedSlot> slot_, _Inout_ auto& obj_) const {
			using target_type = std::remove_cvref_t<decltype(obj_)>;
			serialization::access::Structure<target_type>::hydrate(std::forward<decltype(slot_)>(slot_), obj_);
			return std::addressof(obj_) != nullptr ? Some(obj_) : None;
		}

	public:
		template <class Type_>
		decltype(auto) deserialize(_In_ cref<serialization::LayoutArchiveBase> archive_, auto&&... args_) const {
			using target_type = std::remove_cvref<Type_>;
			auto archive = static_cast<mref<serialization::TypedLayoutArchive<target_type>>>(archive_);
			// TODO: ...
			return None;
		}

		template <class Type_>
		decltype(auto) deserialize(_In_ cref<serialization::StructuredArchive> archive_, auto&&... args_) const {
			using target_type = std::remove_cvref_t<Type_>;
			auto* obj = serialization::access::Structure<target_type>::deserialize(
				archive_.getRootSlot(),
				std::forward<decltype(args_)>(args_)...
			);
			return static_cast<bool>(obj) ? Some(obj) : None;
		}

		template <class Type_>
		decltype(auto) deserialize(_In_ cref<serialization::RecordScopedSlot> slot_, auto&&... args_) const {
			using target_type = std::remove_cvref_t<Type_>;
			auto* obj = serialization::access::Structure<target_type>::deserialize(
				std::forward<decltype(slot_)>(slot_),
				std::forward<decltype(args_)>(args_)...
			);
			return static_cast<bool>(obj) ? Some(obj) : None;
		}
	};
}
