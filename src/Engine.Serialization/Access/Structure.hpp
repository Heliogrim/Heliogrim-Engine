#pragma once

#include <concepts>
#include <type_traits>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Meta/Type.hpp>

#include "../Scheme/Selector.hpp"
#include "../Structure/RecordScopedSlot.hpp"

namespace hg::engine::serialization::access {
	template <class Type_, class Derived_>
	class BaseStructure {
	public:
		using this_type = BaseStructure<Type_, Derived_>;

		using derived_type = Derived_;
		using target_type = Type_;

	protected:
		struct NewDeleteAllocator {
			template <CompleteType TargetType_ = target_type>
			[[nodiscard]] constexpr TargetType_* alloc(auto&&... args_) const
				requires std::is_constructible_v<TargetType_, decltype(args_)...> {
				return new TargetType_(std::forward<decltype(args_)>(args_)...);
			}
		};

	protected:
		template <CompleteType TargetType_ = target_type>
		[[nodiscard]] static TargetType_* instantiate(
			_Inout_ auto&& allocator_,
			auto&&... args_
		) noexcept {
			static_assert(std::derived_from<Derived_, this_type>);

			auto* next = std::forward<decltype(allocator_)>(allocator_).template alloc<TargetType_>(
				std::forward<decltype(args_)>(args_)...
			);
			return next;
		}

		template <
			CompleteType TargetType_ = target_type,
			typename Allocator_ = NewDeleteAllocator,
			typename... Args_
		>
		[[nodiscard]] static TargetType_* deserialize(
			_In_ cref<RecordScopedSlot> slot_,
			Allocator_&& allocator_ = Allocator_ {},
			Args_&&... args_
		) {
			static_assert(std::derived_from<Derived_, this_type>);

			auto* target = derived_type::template instantiate<TargetType_>(
				std::forward<decltype(allocator_)>(allocator_),
				std::forward<Args_>(args_)...
			);
			derived_type::hydrate(*target, std::move(slot_));
			return target;
		}
	};

	/**/

	template <class Type_>
	class StructureObsolete :
		public BaseStructure<Type_, StructureObsolete<Type_>> {
	public:
		using this_type = StructureObsolete<Type_>;
		using base_type = BaseStructure<Type_, this_type>;

		using NewDeleteAllocator = typename base_type::NewDeleteAllocator;

	public:
		/**
		 * Serialization Functions
		 */

		//
		static void serialize(_In_ cref<Type_> self_, _Inout_ mref<RecordScopedSlot> slot_);

	public:
		/**
		 * Deserialization Functions
		 */

		//
		static void hydrate(_Inout_ ref<Type_> target_, _In_ cref<RecordScopedSlot> slot_);

		using base_type::instantiate;
		using base_type::deserialize;
	};

	/**/

	template <class Type_, class Scheme_ = typename SelectScheme<Type_>::default_type>
	class Structure {
	public:
		using this_type = Structure<Type_, Scheme_>;

	public:
		static void serialize(_In_ cref<Type_> self_, _Inout_ mref<StructScopedSlot> record_);

		static void hydrate(_In_ cref<StructScopedSlot> record_, _Inout_ ref<Type_> target_);

	public:
		[[nodiscard]] static decltype(auto) instantiate(auto&&... args_) noexcept
			requires CompleteType<Scheme_> && requires {
				{ Scheme_::template instantiate<this_type>(std::forward<decltype(args_)>(args_)...) };
			} {
			return Scheme_::template instantiate<this_type>(std::forward<decltype(args_)>(args_)...);
		}

		[[nodiscard]] static decltype(auto) deserialize(cref<RecordScopedSlot> slot_, auto&&... args_) noexcept
			requires CompleteType<Scheme_> && requires {
				{ Scheme_::template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...) };
			} {
			return Scheme_::template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...);
		}

	public:
		[[nodiscard]] static decltype(auto) instantiate(auto&&... args_) noexcept
			requires CompleteType<Scheme_> && std::is_empty_v<Scheme_> &&
			std::is_member_function_pointer_v<decltype(&Scheme_::template instantiate<this_type>)> &&
			requires(Scheme_ obj_) {
				{ obj_.template instantiate<this_type>(std::forward<decltype(args_)>(args_)...) };
			} {
			return (Scheme_ {}).template instantiate<this_type>(std::forward<decltype(args_)>(args_)...);
		}

		[[nodiscard]] static decltype(auto) deserialize(cref<RecordScopedSlot> slot_, auto&&... args_) noexcept
			requires CompleteType<Scheme_> && std::is_empty_v<Scheme_> &&
			std::is_member_function_pointer_v<decltype(&Scheme_::template deserialize<this_type>)> &&
			requires(Scheme_ obj_) {
				{ obj_.template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...) };
			} {
			return (Scheme_ {}).template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...);
		}

	public:
		[[nodiscard]] static decltype(auto) instantiate(
			cref<Scheme_> scheme_,
			auto&&... args_
		) noexcept
			requires CompleteType<Scheme_> && (not std::is_empty_v<Scheme_>) &&
			requires {
				{ scheme_.template instantiate<this_type>(std::forward<decltype(args_)>(args_)...) };
			} {
			return scheme_.template instantiate<this_type>(std::forward<decltype(args_)>(args_)...);
		}

		[[nodiscard]] static decltype(auto) deserialize(
			cref<Scheme_> scheme_,
			cref<RecordScopedSlot> slot_,
			auto&&... args_
		) noexcept
			requires CompleteType<Scheme_> && (not std::is_empty_v<Scheme_>) &&
			requires {
				{ scheme_.template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...) };
			} {
			return scheme_.template deserialize<this_type>(slot_, std::forward<decltype(args_)>(args_)...);
		}
	};
}
