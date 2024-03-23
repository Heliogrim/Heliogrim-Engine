#pragma once

#include "ClassMetaBase.hpp"
#include "../Meta/TypedMetaClass.hpp"

namespace hg {
	template <typename>
	class InheritBase;

	/**
	 * @warning Be aware that polymorphic inheritance is fine; Just don't use diamond inheritance !?!
	 */
	template <typename Derived_, typename Base_, typename... Rest_>
	class InheritMeta :
		public Base_,
		public Rest_... {
	public:
		template <typename>
		friend class InheritBase;

		template <typename, typename, typename...>
		friend class InheritMeta;

	public:
		using this_type = InheritMeta<Derived_, Base_, Rest_...>;

		using __inherit_types = typename reflect::__type_list_append<
			reflect::__type_list<Base_, Rest_...>,
			typename reflect::__type_list_aggregate<Base_, Rest_...>::type
		>::type;
		using meta_class = TypedMetaClass<Derived_, __inherit_types>;

	private:
		template <typename Fn_, typename Type_>
		constexpr void __each_base_impl(Fn_&& fn_) noexcept {
			if constexpr (std::derived_from<Type_, InheritBase<Type_>>) {
				std::forward<Fn_>(fn_)(static_cast<ref<ClassMetaBase>>(static_cast<ref<Type_>>(*this)));
			} else if constexpr (std::derived_from<Type_, ClassMetaBase>) {
				Type_::__each_base(std::forward<Fn_>(fn_));
			}
		}

		template <typename Fn_, typename... Types_>
		constexpr void __each_base_call(Fn_&& fn_) noexcept {
			(__each_base_impl<Fn_, Types_>(std::forward<Fn_>(fn_)), ...);
		}

		template <typename Fn_>
		constexpr void __each_base(Fn_&& fn_) noexcept {
			__each_base_call<Fn_, Base_, Rest_...>(std::forward<Fn_>(fn_));
		}

	public:
		constexpr InheritMeta() :
			Base_(),
			Rest_()... {
			__each_base(
				[](ref<ClassMetaBase> base_) noexcept {
					base_._meta = meta_class::get();
				}
			);
		}

		template <typename... Args_>
		constexpr InheritMeta(Args_&&... args_) :
			Base_(std::forward<Args_>(args_)...),
			Rest_()... {
			__each_base(
				[](ref<ClassMetaBase> base_) noexcept {
					base_._meta = meta_class::get();
				}
			);
		}

	private:
		[[nodiscard]] constexpr const __restricted_ptr<const MetaClass> getMetaClassBase() const noexcept {
			return static_cast<const ptr<const Base_>>(this)->getMetaClassBase();
		}

	public:
		/**
		 * Will query for the dynamically stored reference to the related meta object.
		 *
		 * @warning You shall be strictly prevent writing access to the returned data.
		 * @returns A pointer to the compile-time object.
		 */
		[[nodiscard]] const __restricted_ptr<const meta_class> getMetaClass() const noexcept {
			return static_cast<const __restricted_ptr<const meta_class>>(getMetaClassBase());
		}

		/**
		 * Will query statically for the compile-time instantiated meta class object related to this type.
		 *
		 * @warning You shall be strictly prevent writing access to the returned data.
		 * @returns A pointer to the compile-time object.
		 */
		[[nodiscard]] constexpr static const __restricted_ptr<const meta_class> getStaticMetaClass() noexcept {
			return meta_class::get();
		}
	};
}
