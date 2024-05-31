#pragma once
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

#include "ConfigEntry.hpp"
#include "ConfigKey.hpp"

namespace hg::engine::cfg {
	class __declspec(novtable) Provider {
	public:
		constexpr virtual ~Provider() noexcept = default;

	public:
		virtual void add(mref<ConfigEntry> entry_) = 0;

		[[nodiscard]] virtual bool contains(InternalConfigKey key_) const noexcept = 0;

		[[nodiscard]] virtual ConfigValue get(InternalConfigKey key_) const = 0;

		[[nodiscard]] _Success_(return) virtual bool set(InternalConfigKey key_, mref<ConfigValue> next_) noexcept = 0;

		virtual bool remove(InternalConfigKey key_) = 0;

	public:
		[[nodiscard]] virtual bool isConfigType(InternalConfigKey key_, type_id typeId_) const noexcept = 0;
	};

	/**/

	using ProviderId = u64;

	template <class Type_>
	concept ProviderIdGetter = requires(const Type_& obj_) {
		{ obj_.providerId() } -> std::same_as<ProviderId>;
	};

	template <class Type_> requires std::derived_from<Type_, Provider>
	[[nodiscard]] constexpr ProviderId query_provider_id(cref<Type_> provider_) noexcept {
		if constexpr (ProviderIdGetter<Type_>) {
			return provider_.providerId();
		} else {
			return reflect::typeId<Type_>().data;
		}
	}
}
