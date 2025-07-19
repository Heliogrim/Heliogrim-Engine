#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Meta/Type.hpp>

namespace hg {
	class AssetDatabase;

	template <class AssetType_>
	class TypedAssetHandle;
}

namespace hg::engine::assets {
	class Asset;
}

namespace hg {
	class AssetHandle {
	public:
		friend class ::hg::AssetDatabase;

	public:
		using this_type = AssetHandle;

	protected:
		AssetHandle() noexcept;

		AssetHandle(mref<AssetGuid> guid_, ref<const AssetTypeId> type_, mref<Opt<ref<engine::assets::Asset>>> ref_) noexcept;

		AssetHandle(ref<const this_type> other_) noexcept;

	public:
		AssetHandle(mref<this_type> other_) noexcept;

		constexpr ~AssetHandle() = default;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept;

		ref<this_type> operator=(ref<const this_type> other_) noexcept;

	protected:
		AssetGuid _guid;
		AssetTypeId _type;
		Opt<ref<engine::assets::Asset>> _ref;

	public:
		[[nodiscard]] ref<const AssetGuid> getAssetGuid() const noexcept;

		[[nodiscard]] operator ref<const AssetGuid>() const noexcept;

	public:
		[[nodiscard]] ref<const AssetTypeId> getAssetTypeId() const noexcept;

		[[nodiscard]] operator ref<const AssetTypeId>() const noexcept;

	public:
		template <class AssetType_ = ::hg::engine::assets::Asset>
			requires CompleteType<AssetType_> && std::is_same_v<AssetType_, ::hg::engine::assets::Asset>
		[[nodiscard]] Opt<ref<const engine::assets::Asset>> getAssetRef() const noexcept {
			return _ref != None ? Some(*_ref) : None;
		}

	public:
		[[nodiscard]] bool isValid() const noexcept;

		[[nodiscard]] bool operator==(ref<const AssetGuid> guid_) const noexcept;

		[[nodiscard]] bool operator==(ref<const AssetTypeId> type_) const noexcept;

		[[nodiscard]] bool operator==(ref<const AssetHandle> handle_) const noexcept;

		[[nodiscard]] bool operator!=(ref<const AssetGuid> guid_) const noexcept;

		[[nodiscard]] bool operator!=(ref<const AssetTypeId> type_) const noexcept;

		[[nodiscard]] bool operator!=(ref<const AssetHandle> handle_) const noexcept;

		[[nodiscard]] std::strong_ordering operator<=>(ref<const AssetHandle> other_) const noexcept;

	public:
		template <class AssetType_> requires CompleteType<TypedAssetHandle<AssetType_>>
		[[nodiscard]] Opt<TypedAssetHandle<AssetType_>> asTyped() const noexcept;
	};
}
