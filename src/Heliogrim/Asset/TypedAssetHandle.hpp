#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "AssetHandle.hpp"

namespace hg {
	template <class AssetType_>
	class TypedAssetHandle :
		public AssetHandle {
	public:
		friend class AssetHandle;
		friend class AssetDatabase;

	public:
		using this_type = TypedAssetHandle<AssetType_>;
		using asset_type = AssetType_;

	private:
		template <typename Type_ = AssetType_>
			requires CompleteType<Type_> && std::is_same_v<Type_, AssetType_> && std::derived_from<Type_, engine::assets::Asset>
		TypedAssetHandle(mref<AssetGuid> guid_, ref<const AssetTypeId> type_, mref<Opt<ref<Type_>>> ref_) :
			AssetHandle(::hg::move(guid_), type_, { ref_.value() }) {}

		template <typename Type_ = AssetType_>
			requires CompleteType<Type_> && std::is_same_v<Type_, AssetType_> && std::derived_from<Type_, engine::assets::Asset>
		TypedAssetHandle(ref<const AssetHandle> handle_) :
			AssetHandle(handle_) {}

	public:
		template <typename Type_ = AssetType_>
			requires CompleteType<Type_> && std::is_same_v<Type_, AssetType_>
		TypedAssetHandle() noexcept :
			AssetHandle() {}

		template <typename Type_ = AssetType_>
			requires CompleteType<Type_> && std::is_same_v<Type_, AssetType_> && std::derived_from<Type_, engine::assets::Asset>
		[[nodiscard]] Opt<ref<const Type_>> getAssetRef() const noexcept {
			return _ref != None ? Some(static_cast<ref<const Type_>>(*_ref)) : None;
		}

		template <typename Type_ = AssetType_>
			requires CompleteType<Type_> && std::is_same_v<Type_, AssetType_>
		[[nodiscard]] static AssetTypeId getStaticTypeId() noexcept;
	};

	template <class AssetType_> requires CompleteType<TypedAssetHandle<AssetType_>>
	Opt<TypedAssetHandle<AssetType_>> AssetHandle::asTyped() const noexcept {
		using target_type = TypedAssetHandle<AssetType_>;

		const auto tmpTypeId = target_type::template getStaticTypeId<AssetType_>();
		// Error: We may need to add a constructor overload using const-lv-ref for input instead of rv-ref
		auto tmp = target_type {
			clone(_guid),
			tmpTypeId,
			_ref != None ? Some(static_cast<ref<AssetType_>>(const_cast<ref<engine::assets::Asset>>(*_ref))) : None
		};
		return tmp.isValid() ? Some(::hg::move(tmp)) : None;
	}
}
