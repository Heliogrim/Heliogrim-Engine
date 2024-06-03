#pragma once

#include "../Url/IUrlComp.hpp"

namespace hg::engine::storage {
	/**/

	template <class Derived_>
	class IStorageDescriptor;

	template <class Type_>
	concept IsStorageDescriptorComp = std::derived_from<Type_, IStorageDescriptor<Type_>> &&
		requires(const Type_& obj_) {
			{ obj_.targetScheme() } -> std::same_as<UrlScheme>;
			{ obj_.valid() } -> std::same_as<bool>;
		};

	/**/

	template <class Derived_>
	class __declspec(novtable) IStorageDescriptor {
	protected:
		// Attention: We may need to defer the constructor instantiation...
		constexpr IStorageDescriptor() noexcept {
			static_assert(IsStorageDescriptorComp<Derived_>);
		}

	public:
		constexpr IStorageDescriptor(const IStorageDescriptor&) noexcept = default;

		constexpr IStorageDescriptor(IStorageDescriptor&&) noexcept = default;

		constexpr ~IStorageDescriptor() noexcept = default;
	};
}
