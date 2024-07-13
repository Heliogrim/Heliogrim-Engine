#pragma once
#include "InheritMeta.hpp"

namespace hg {
	template <typename Derived_>
	class InheritBase :
		public ClassMetaBase {
	public:
		template <typename>
		friend class InheritBase;

		template <typename, typename, typename...>
		friend class InheritMeta;

	public:
		using this_type = InheritBase<Derived_>;

		using __inherit_types = reflect::__type_list<ClassMetaBase>;

		/**
		 * Meta-Class Substitution
		 *	Note: We need to defer the template instantiation, as Derived_ is incomplete
		 *			while we instantiate the inheritance helper class.
		 */
		template <typename ThisType_ = Derived_>
		using meta_class = TypedMetaClass<ThisType_, __inherit_types>;

		template <typename ThisType_ = Derived_>
		using meta_class_ret_type = const __restricted_ptr<const meta_class<ThisType_>>;

	public:
		constexpr InheritBase() noexcept :
			ClassMetaBase() {
			ClassMetaBase::_meta = meta_class<>::get();
		}

	private:
		START_SUPPRESS_WARNINGS
		// NOLINTBEGIN(*-const-return-type)

		[[nodiscard]] constexpr const __restricted_ptr<const MetaClass> getMetaClassBase() const noexcept {
			return ClassMetaBase::getMetaClass();
		}

		// NOLINTEND(*-const-return-type)
		STOP_SUPPRESS_WARNINGS

	public:
		/**
		 * Will query for the dynamically stored reference to the related meta object.
		 *
		 * @warning You shall be strictly prevent writing access to the returned data.
		 * @returns A pointer to the compile-time object.
		 */
		// NOLINTBEGIN(*-const-return-type)
		template <typename DerivedType_ = Derived_>
		[[nodiscard]] meta_class_ret_type<DerivedType_> getMetaClass() const noexcept {
			// NOLINTEND(*-const-return-type)
			return static_cast<meta_class_ret_type<DerivedType_>>(ClassMetaBase::getMetaClass());
		}

		/**
		 * Will query statically for the compile-time instantiated meta class object related to this type.
		 *
		 * @warning You shall be strictly prevent writing access to the returned data.
		 * @returns A pointer to the compile-time object.
		 */
		// NOLINTBEGIN(*-const-return-type)
		template <typename DerivedType_ = Derived_>
		[[nodiscard]] constexpr static meta_class_ret_type<DerivedType_> getStaticMetaClass() noexcept {
			// NOLINTEND(*-const-return-type)
			return meta_class<>::get();
		}
	};
}
