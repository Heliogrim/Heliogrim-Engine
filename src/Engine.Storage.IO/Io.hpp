#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::storage {
	template <typename ManagedType_>
	struct Io {
		using this_type = Io<ManagedType_>;

		std::add_pointer_t<std::remove_reference_t<std::remove_pointer_t<ManagedType_>>> managed;

		[[nodiscard]] constexpr bool fullyLoaded() const noexcept {
			return true;
			return false;
		}

		[[nodiscard]] ref<ManagedType_> fully() const {
			// TODO: await resource to be fully loaded and backed by memory / instantiated
			::hg::assertrt(fullyLoaded());
			return *managed;
		}

		[[nodiscard]] ptr<ManagedType_> operator->() const {
			return std::addressof(fully());
		}

		[[nodiscard]] ptr<ManagedType_> operator->() {
			return std::addressof(fully());
		}

	public:
		/* Meta Queries */

		/**
		 * Check whether the io handle can read from the underlying data section.
		 *
		 * @details This corresponds to the basic ability to immutably access the target space, which includes
		 *  the minimal requirement for sequential reading.
		 *
		 * @returns A bool whether this is supported.
		 */
		[[nodiscard]] constexpr bool readable() const noexcept;

		/**
		 * Check whether the io handle can randomly read from the underlying data section.
		 *
		 * @details This is commonly supported for in-memory operations and file io operations, but may indicate huge
		 *  differences related to latency and access speed.
		 *
		 * @returns A bool whether this is supported.
		 */
		[[nodiscard]] constexpr bool randomReadable() const noexcept;

		/**
		 * Check whether the io handle can mutate the underlying target section.
		 *
		 * @details This corresponds to the basic ability to mutate the target space, which includes the minimal
		 *  requirement for sequential writing.
		 *
		 * @returns A bool whether this is supported.
		 */
		[[nodiscard]] constexpr bool writable() const noexcept;

		/**
		 * Check whether the io handle can overwrite sections which were sequentially written beforehand.
		 *
		 * @details This is especially required when dealing this writable resources manipulating pre-written header
		 *	sections, as file io operations may not allow random access write operations due to inconsistent offsets.
		 *
		 * @returns A bool whether this is supported.
		 */
		[[nodiscard]] constexpr bool rewritable() const noexcept;

		/**
		 * Check whether the io handle can randomly write into the underlying target section.
		 *
		 * @details This is commonly supported for pre-allocated memory and rarely supported for file io operations,
		 *	as the underlying system may have to clean up the fragmented target space.
		 *
		 * @returns A bool whether this is supported.
		 */
		[[nodiscard]] constexpr bool randomWritable() const noexcept;
	};
}

#include <Engine.Asserts/Todo.hpp>

// TODO: Move Implementation
namespace hg::engine::storage {
	template <typename ManagedType_>
	constexpr bool Io<ManagedType_>::readable() const noexcept {
		return true;

		::hg::todo_panic();
		return false;
	}

	template <typename ManagedType_>
	constexpr bool Io<ManagedType_>::randomReadable() const noexcept {
		return true;

		::hg::todo_panic();
		return false;
	}

	template <typename ManagedType_>
	constexpr bool Io<ManagedType_>::writable() const noexcept {
		return true;

		::hg::todo_panic();
		return false;
	}

	template <typename ManagedType_>
	constexpr bool Io<ManagedType_>::rewritable() const noexcept {
		return true;

		::hg::todo_panic();
		return false;
	}

	template <typename ManagedType_>
	constexpr bool Io<ManagedType_>::randomWritable() const noexcept {
		return true;

		::hg::todo_panic();
		return false;
	}
}
