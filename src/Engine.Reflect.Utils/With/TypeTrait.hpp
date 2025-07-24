#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Reflect/TypeId.hpp>

namespace hg::meta {
	template <typename Type_>
	struct TypeTrait {
		constexpr TypeTrait() noexcept = default;

		template <typename Ux_>
		constexpr explicit TypeTrait(TypeTrait<Ux_>&& other_) noexcept :
			typeId(::hg::move(other_.typeId)) {}

		constexpr TypeTrait(const TypeTrait&) noexcept = default;

		constexpr TypeTrait(TypeTrait&&) noexcept = default;

		constexpr ~TypeTrait() noexcept = default;

		template <typename Ux_>
		TypeTrait& operator=(TypeTrait<Ux_>&& other_) noexcept {
			typeId = ::hg::move(other_.typeId);
			return *this;
		}

		TypeTrait& operator=(mref<TypeTrait>) noexcept = default;

		TypeTrait& operator=(cref<TypeTrait>) noexcept = default;

		type_id typeId = ::hg::refl::TypeId<Type_>();

		[[nodiscard]] constexpr bool valid() const noexcept {
			return typeId == ::hg::refl::TypeId<Type_>();
		}
	};
}
