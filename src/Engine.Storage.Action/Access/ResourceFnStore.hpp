#pragma once

#include <concepts>

#include "IoResource.hpp"

namespace hg::engine::storage {
	template <typename MutValFn_, typename ConstValFn_ = MutValFn_>
	struct ResourceFnStore;

	/**/

	template <typename AllValFn_>
	struct ResourceFnStore<AllValFn_, AllValFn_> {
		using mut_ret_type = std::invoke_result_t<AllValFn_, ResourceAccessMode>;
		using mut_val_type = std::remove_reference_t<mut_ret_type>;

		using const_ret_type = cref<std::remove_cvref_t<mut_ret_type>>;
		using const_val_type = std::add_const_t<std::remove_cvref_t<const_ret_type>>;

		using ior_type = IoResource<mut_val_type, const_val_type>;

		AllValFn_ valueFn;

		[[nodiscard]] mut_ret_type resolveMutable(
			[[maybe_unused]] cref<ior_type> ior_,
			const ResourceAccessMode mode_
		) const noexcept {
			return valueFn(mode_);
		}

		[[nodiscard]] const_ret_type resolveConstant(
			[[maybe_unused]] cref<ior_type> ior_,
			const ResourceAccessMode mode_
		) const noexcept {
			return valueFn(mode_);
		}
	};

	template <typename MutValFn_, typename ConstValFn_> requires (not std::is_same_v<MutValFn_, ConstValFn_>)
	struct ResourceFnStore<MutValFn_, ConstValFn_> {
		using mut_ret_type = std::invoke_result_t<MutValFn_, ResourceAccessMode>;
		using mut_val_type = std::remove_reference_t<mut_ret_type>;

		using const_ret_type = std::invoke_result_t<ConstValFn_, ResourceAccessMode>;
		using const_val_type = std::remove_reference_t<const_ret_type>;

		using ior_type = IoResource<mut_val_type, const_val_type>;

		MutValFn_ mutValFn;
		ConstValFn_ constValFn;

		[[nodiscard]] mut_ret_type resolveMutable(
			[[maybe_unused]] cref<ior_type> ior_,
			const ResourceAccessMode mode_
		) const noexcept {
			return mutValFn(mode_);
		}

		[[nodiscard]] const_ret_type resolveConstant(
			[[maybe_unused]] cref<ior_type> ior_,
			const ResourceAccessMode mode_
		) const noexcept {
			return constValFn(mode_);
		}
	};
}
