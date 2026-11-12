#pragma once

#include <span>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "AccessResources.hpp"

namespace hg::engine::scheduler::graph {
	/**
	 * The identity of one registered access resource.
	 *
	 * The only way to make a key is `AccessKey::of<T>()` at compile time with `T` in the closed set of
	 * `AccessResources.hpp`; there is no empty key and no key from a raw `type_id` or number.
	 */
	class AccessKey final {
	public:
		using this_type = AccessKey;

	private:
		constexpr explicit AccessKey(const type_id id_) noexcept :
			_id(id_) {}

	public:
		AccessKey() = delete;

	public:
		[[nodiscard]] constexpr bool operator==(ref<const this_type>) const noexcept = default;

	private:
		type_id _id;

	public:
		template <AccessResource Type_>
		[[nodiscard]] static consteval this_type of() noexcept {
			return this_type { refl::ctid<Type_>() };
		}

		[[nodiscard]] constexpr type_id id() const noexcept {
			return _id;
		}
	};

	enum class Lane : u8 {
		eCritical,
		eNormal,
		eBackground
	};

	struct StageDecl {
		string_view name;
		std::span<const AccessKey> reads;
		std::span<const AccessKey> writes;
		Lane lane = Lane::eNormal;
		std::span<const string_view> after;
		std::span<const string_view> before;
	};
}
