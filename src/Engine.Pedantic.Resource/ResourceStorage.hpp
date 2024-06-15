#pragma once

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	template <typename ManagedType_>
	class ResourceAccessor;

	/**/

	template <typename ManagedType_>
	struct ResourceInternalStorage {
		using managed_type = std::remove_const_t<ManagedType_>;
		mutable managed_type value;
	};

	template <typename ManagedType_>
	struct ResourceInternalStorage<const ManagedType_> {
		using managed_type = std::add_const_t<ManagedType_>;
		std::remove_const_t<managed_type> value;
	};

	template <typename ManagedType_>
	struct ResourceStorage :
		public ResourceInternalStorage<ManagedType_> {
	public:
		using this_type = ResourceStorage<ManagedType_>;

		using internal_type = ResourceInternalStorage<ManagedType_>;
		using managed_type = typename internal_type::managed_type;

	public:
		[[nodiscard]] constexpr cref<ManagedType_> getConstValueReference() const noexcept {
			return internal_type::value;
		}

		[[nodiscard]] constexpr ref<ManagedType_> getValueReference() const noexcept
			requires (not std::is_const_v<managed_type>) {
			return internal_type::value;
		}

		[[nodiscard]] constexpr cref<ManagedType_> getValueReference() const noexcept
			requires (std::is_const_v<managed_type>) {
			return internal_type::value;
		}

	public:
		[[nodiscard]] constexpr static this_type from(mref<ResourceAccessor<ManagedType_>> source_)
			requires (not std::is_const_v<managed_type>) && std::is_nothrow_move_constructible_v<managed_type> {
			::hg::assertd(source_.valid());
			return this_type { std::move(source_.get(std::nothrow)) };
		}
	};
}
