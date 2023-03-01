#pragma once

#include "../__macro.hpp"

#include <cstdlib>
#include <memory>

namespace hg::engine::memory {
    template <class Ty, size_t Align>
    class aligned_allocator {
    public:
        using size_type = _STD size_t;
        using difference_type = _STD ptrdiff_t;

        using value_type = Ty;
        using reference_type = Ty&;
        using pointer_type = Ty*;
        using const_reference_type = const Ty&;
        using const_pointer_type = const Ty*;

        using this_type = aligned_allocator<Ty, Align>;

        template <class UnionTy>
        struct rebind {
            typedef aligned_allocator<UnionTy, Align> other;
        };

        /**
         * Default constructor
         *
         * @author Julius
         * @date 15.11.2020
         */
        aligned_allocator() noexcept = default;

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  other_ The other.
         */
        aligned_allocator(const aligned_allocator& other_) = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @tparam UnionTy Type of the union ty.
         */
        template <typename UnionTy>
        aligned_allocator(const aligned_allocator<UnionTy, Align>&) noexcept {};

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.11.2020
         */
        ~aligned_allocator() = default;

        /**
         * Address the given value
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param [in,out] value_ The value.
         *
         * @returns A pointer_type.
         */
        [[nodiscard]] FORCE_INLINE pointer_type address(reference_type value_) const noexcept {
            return _STD addressof(value_);
        }

        /**
         * Address the given value
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  value_ The value.
         *
         * @returns A const_pointer_type.
         */
        [[nodiscard]] FORCE_INLINE const_pointer_type address(const_reference_type value_) const noexcept {
            return _STD addressof(value_);
        }

        /**
         * Deallocates
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  ptr_ The pointer.
         * @param  count_ Number of.
         */
        void deallocate(pointer_type ptr_, const size_type count_) noexcept {
            free(ptr_);
        }

        /**
         * Allocates the given count
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  count_ Number of.
         *
         * @returns A pointer_type.
         */
        [[nodiscard]] pointer_type allocate(_CRT_GUARDOVERFLOW const size_t count_, const void* = nullptr) {
            static_assert(Align >= sizeof(void*));

            if (count_ == 0) {
                return nullptr;
            }

            void* ptr = nullptr;
            const auto size = sizeof(value_type) * count_;

            #if defined(ENV_MSVC)
            ptr = _aligned_malloc(size, Align);
            #elif defined(ENV_GCC)
			if (posix_memalign(&ptr, Align, size) != 0) {
				ptr = nullptr;
			}
            #endif

            if (ptr == nullptr) {
                throw _STD bad_alloc();
            }

            return static_cast<pointer_type>(ptr);
        }
    };
}
