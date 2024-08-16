#pragma once

#include <stdexcept>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Hash.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

#include "ConfigEntry.hpp"
#include "ConfigKey.hpp"
#include "ConfigValue.hpp"
#include "Provider.hpp"

namespace hg::engine::cfg {
	class ConfigValueTypeMismatchError final : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class ConfigInitError final : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class ConfigUpdateError final : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};
}

namespace hg::engine {
	class Config {
	public:
		using this_type = Config;

	public:
		constexpr Config() noexcept = default;

		template <class... InitialProvider_>
		constexpr explicit Config(uptr<InitialProvider_>&&... initial_) noexcept :
			Config() {
			(registerProvider(std::forward<decltype(initial_)>(initial_)), ...);
		}

		Config(mref<this_type>) = delete;

		Config(cref<this_type>) = delete;

		constexpr ~Config() noexcept = default;

	private:
		Vector<std::pair<cfg::ProviderId, uptr<cfg::Provider>>> _providers;

	private:
		[[nodiscard]] tl::expected<void, cfg::ConfigInitError> init(mref<cfg::ConfigEntry> entry_);

		[[nodiscard]] tl::expected<void, cfg::ConfigInitError> init(
			mref<cfg::ConfigEntry> entry_,
			cfg::ProviderId providerId_
		);

		[[nodiscard]] tl::optional<cfg::ConfigValue> get(cref<cfg::InternalConfigKey> key_) const noexcept;

		[[nodiscard]] tl::expected<void, cfg::ConfigUpdateError> update(
			cref<cfg::InternalConfigKey> key_,
			mref<cfg::ConfigValue> next_
		) noexcept;

		bool drop(cref<cfg::InternalConfigKey> key_) noexcept;

		template <typename KeyType_> requires cfg::IsConfigKey<std::remove_cvref_t<KeyType_>>
		[[nodiscard]] constexpr cfg::InternalConfigKey convertKey(KeyType_&& key_) const noexcept {
			using CheckType = std::remove_cvref_t<KeyType_>;
			if constexpr (std::is_same_v<CheckType, CompileString>) {
				return std::forward<KeyType_>(key_).hash();

			} else if constexpr (std::is_same_v<CheckType, StringView>) {
				if (std::is_constant_evaluated()) {
					return CompileString { &key_.front(), &key_.back() }.hash();
				}
				return hash::fnv1a(std::forward<KeyType_>(key_));

			} else if constexpr (std::is_enum_v<CheckType>) {
				const auto enumTypeId = ::hg::reflect::typeId<CheckType>();
				const auto enumValInt = std::bit_cast<std::underlying_type_t<CheckType>>(std::forward<KeyType_>(key_));

				auto combined = enumTypeId.data;
				hash::hashCombine(combined, static_cast<decltype(enumTypeId.data)>(enumValInt));
				return combined;

			} else {
				static_assert(std::is_enum_v<CheckType>);
				std::unreachable();
			}
		}

	public:
		/**
		 *
		 * @note This variation will push the provided key-value pair onto the lowest
		 *	layer registered to the config holder object. The operation will fail if
		 *	there is not valid provider layer or the key is already known to the lowest layer.
		 *
		 * @tparam KeyType_ Template parameter type of the key argument
		 * @tparam ValueType_ Template parameter type of the value argument
		 * @param key_ Layer unique identifier to store the config value.
		 * @param value_ Config value to store.
		 * @returns An expected object to check for a failed operation.
		 */
		template <typename KeyType_, typename ValueType_> requires
			cfg::IsConfigKey<std::remove_cvref_t<KeyType_>> &&
			cfg::IsConfigValue<std::remove_cvref_t<ValueType_>>
		[[nodiscard]] tl::expected<void, cfg::ConfigInitError> init(KeyType_&& key_, ValueType_&& value_) noexcept {
			return init(
				cfg::ConfigEntry {
					reflect::typeId<std::remove_cvref_t<ValueType_>>(),
					convertKey(std::forward<KeyType_>(key_)),
					{ std::forward<ValueType_>(value_) }
				}
			);
		}

		/**
		 *
		 * @note This variation will store the provided key-value pair into the associated provider by the
		 *	provider id. The operation will fail if the provider can not be queried or the key is already known.
		 *
		 * @tparam KeyType_ Template parameter type of the key argument
		 * @tparam ValueType_ Template parameter type of the value argument
		 * @param key_ Provider unique identifier to store the config value.
		 * @param value_ Config value to store.
		 * @param providerId_ The unique identifier of the provider where to store the key-value pair.
		 * @returns An expected object to check for a failed operation.
		 */
		template <typename KeyType_, typename ValueType_> requires
			cfg::IsConfigKey<std::remove_cvref_t<KeyType_>> &&
			cfg::IsConfigValue<std::remove_cvref_t<ValueType_>>
		[[nodiscard]] tl::expected<void, cfg::ConfigInitError> init(
			KeyType_&& key_,
			ValueType_&& value_,
			cfg::ProviderId providerId_
		) noexcept {
			return init(
				cfg::ConfigEntry {
					reflect::typeId<std::remove_cvref_t<ValueType_>>(),
					convertKey(std::forward<KeyType_>(key_)),
					{ std::forward<ValueType_>(value_) }
				},
				providerId_
			);
		}

		template <typename KeyType_> requires cfg::IsConfigKey<std::remove_cvref_t<KeyType_>>
		[[nodiscard]] tl::optional<cfg::ConfigValue> get(KeyType_&& key_) const noexcept {
			return get(convertKey(std::forward<KeyType_>(key_)));
		}

		template <cfg::IsConfigValue ValueType_, typename KeyType_>
			requires cfg::IsConfigKey<std::remove_cvref_t<KeyType_>>
		[[nodiscard]] tl::expected<tl::optional<ValueType_>, cfg::ConfigValueTypeMismatchError> getTyped(
			KeyType_&& key_
		) const noexcept {
			auto stored = get(convertKey(std::forward<KeyType_>(key_)));
			if (stored.has_value() && std::holds_alternative<ValueType_>(stored.value())) {
				return tl::expected<tl::optional<ValueType_>, cfg::ConfigValueTypeMismatchError> {
					std::move(stored).map([](auto&& val_) { return std::get<ValueType_>(val_); })
				};
			}
			return tl::make_unexpected(cfg::ConfigValueTypeMismatchError { "" });
		}

		template <cfg::IsConfigValue ValueType_, typename KeyType_>
			requires cfg::IsConfigKey<std::remove_cvref_t<KeyType_>>
		[[nodiscard]] tl::expected<tl::optional<ValueType_>, cfg::ConfigValueTypeMismatchError> getOrDefault(
			KeyType_&& key_,
			cref<ValueType_> default_
		) noexcept {
			return get<ValueType_>(std::forward<KeyType_>(key_)).map(
				[&default_](tl::optional<ValueType_>&& val_) { return val_.or_else(default_); }
			);
		}

		template <typename KeyType_, typename ValueType_> requires
			cfg::IsConfigKey<std::remove_cvref_t<KeyType_>> &&
			cfg::IsConfigValue<std::remove_cvref_t<ValueType_>>
		[[nodiscard]] tl::expected<void, cfg::ConfigUpdateError> update(KeyType_&& key_, ValueType_&& next_) noexcept {
			return update(
				convertKey(std::forward<KeyType_>(key_)),
				cfg::ConfigValue { std::forward<ValueType_>(next_) }
			);
		}

		template <typename KeyType_> requires cfg::IsConfigKey<std::remove_cvref_t<KeyType_>>
		bool drop(KeyType_&& key_) noexcept {
			return drop(convertKey(std::forward<KeyType_>(key_)));
		}

	private:
		ref<cfg::Provider> registerProvider(
			mref<cfg::ProviderId> providerId_,
			mref<uptr<cfg::Provider>> provider_
		);

		uptr<cfg::Provider> overwriteProvider(
			mref<cfg::ProviderId> providerId_,
			mref<uptr<cfg::Provider>> next_
		);

		uptr<cfg::Provider> unregisterProvider(
			mref<cfg::ProviderId> providerId_,
			cref<cfg::Provider> provider_
		);

	public:
		template <class ProviderType_> requires std::derived_from<ProviderType_, cfg::Provider>
		ref<ProviderType_> registerProvider(_In_ mref<uptr<ProviderType_>> provider_) {
			auto providerId = cfg::query_provider_id<ProviderType_>(*provider_);
			return static_cast<ProviderType_&>(registerProvider(std::move(providerId), std::move(provider_)));
		}

		template <class ProviderType_> requires std::derived_from<ProviderType_, cfg::Provider>
		uptr<cfg::Provider> overwriteProvider(_In_ mref<uptr<ProviderType_>> next_) {
			auto providerId = cfg::query_provider_id<ProviderType_>(*next_);
			return replaceProvider(std::move(providerId), std::move(next_));
		}

		template <class ProviderType_> requires std::derived_from<ProviderType_, cfg::Provider>
		uptr<cfg::Provider> unregisterProvider(cref<ProviderType_> provider_) {
			auto providerId = cfg::query_provider_id<ProviderType_>(provider_);
			return unregisterProvider(std::move(providerId), provider_);
		}
	};
}
