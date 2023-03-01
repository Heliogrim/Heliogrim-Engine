#pragma once

#include "../../Wrapper.hpp"

namespace hg::hopscotch {
    using hash_truncate_type = _STD uint_least32_t;

    /**
     * A hopscotch bucket hash storage object.
     *
     * @author Julius
     * @date 04.10.2021
     *
     * @tparam Store_ Type of the store.
     */
    template <bool Store_>
    class hopscotch_bucket_hash {
    public:
        /**
         * Query if hash is equal
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool hash_equal(const _STD size_t) const noexcept {
            return true;
        }

        /**
         * Gets the truncated hash value
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A hash_truncate_type.
         */
        [[nodiscard]] hash_truncate_type truncated_hash() const noexcept {
            return 0;
        }

    protected:
        /**
         * Copies the hash from given bucket
         *
         * @author Julius
         * @date 04.10.2021
         */
        void copy_hash(cref<hopscotch_bucket_hash>) noexcept {}

        /**
         * Sets the hash value to given value
         *
         * @author Julius
         * @date 04.10.2021
         */
        void set_hash(hash_truncate_type) noexcept {}
    };

    /**
     * A hopscotch bucket hash storage object with active hash store.
     *
     * @author Julius
     * @date 04.10.2021
     */
    template <>
    class hopscotch_bucket_hash<true> {
    public:
        [[nodiscard]] bool hash_equal(const _STD size_t hash_) const noexcept {
            return _hash_trunc == static_cast<hash_truncate_type>(hash_);
        }

        [[nodiscard]] hash_truncate_type truncated_hash() const noexcept {
            return _hash_trunc;
        }

    protected:
        void copy_hash(cref<hopscotch_bucket_hash> other_) noexcept {
            _hash_trunc = other_._hash_trunc;
        }

        void set_hash(hash_truncate_type hash_) noexcept {
            _hash_trunc = hash_;
        }

    private:
        hash_truncate_type _hash_trunc;
    };
}
