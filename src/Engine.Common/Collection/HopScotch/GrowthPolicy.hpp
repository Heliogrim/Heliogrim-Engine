#pragma once

#include <concepts>
#include <memory>

#include "../../__macro.hpp"
#include "../../Types.hpp"

namespace ember::hopscotch {
    /**
     * Concept
     */
    template <typename PolicyType_>
    concept IsGrowthPolicy = requires(const PolicyType_ obj) {
            { obj.next_bucket_count() } -> std::same_as<_STD size_t>;
        } && requires(const PolicyType_ obj, const _STD size_t hash) {
            { obj.bucket_idx(hash) } -> std::same_as<_STD size_t>;
        } && requires(const _STD size_t minSize) {
            { PolicyType_(minSize) } -> std::same_as<PolicyType_>;
        };

    template <_STD size_t GrowthFactor = 2uLL>
    struct exp2_growth_policy {
        /**
             * Constructor
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  min_ The minimum.
             */
        explicit exp2_growth_policy(const _STD size_t min_) :
            _mask(fast_ceil2(min_) - 1u) {}

        /**
         * Bucket index
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  hash_ The hash.
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t bucket_idx(const _STD size_t hash_) const noexcept {
            return hash_ & _mask;
        }

        /**
         * Next bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t next_bucket_count() const {
            return (_mask + 1) * GrowthFactor;
        }

        /**
         * Maximum bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t max_bucket_count() const {
            return (limits<size_t>::max / 2) + 1uLL;
        }

        /**
         * Clears this  to its blank/initial state
         *
         * @author Julius
         * @date 29.10.2020
         */
        void clear() noexcept {
            _mask = 0;
        }

    private:
        _STD size_t _mask;

        /**
         * Fast ceiling to pow of 2 value
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns A size_t.
         */
        static _STD size_t fast_ceil2(_STD size_t value_) {
            if (ip2(value_))
                return value_;

            if (value_ == 0)
                return 1;

            --value_;
            for (_STD size_t i = 1; i < sizeof(_STD size_t) * CHAR_BIT; i <<= 1) {
                value_ |= value_ >> i;
            }

            return value_ + 1u;
        }

        /**
         * Check if value_ is power of 2
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns True if it succeeds, false if it fails.
         */
        static constexpr bool ip2(const _STD size_t value_) {
            return value_ != 0 && (value_ & (value_ - 1u)) == 0;
        }

        static_assert(ip2(GrowthFactor), "GrowthFactor have to be a power of two");
        static_assert(GrowthFactor >= 2uLL, "GrowthFactor have to be greater or equal to 2");
    };

    static_assert(IsGrowthPolicy<exp2_growth_policy<>>);

    template <_STD size_t GrowthFactor = 2uLL, typename CalcType_ = float>
    struct log2_growth_policy {
        /**
         * Constructor
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  min_ The minimum.
         */
        explicit log2_growth_policy(const _STD size_t min_) :
            _mask(MAX(min_, 2)) {}

        /**
         * Bucket index
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  hash_ The hash.
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t bucket_idx(const _STD size_t hash_) const noexcept {
            return hash_ & _mask;
        }

        /**
         * Next bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t next_bucket_count() const {
            CalcType_ next { _STD log2(static_cast<CalcType_>(_mask)) };
            return _mask + static_cast<CalcType_>(next) * GrowthFactor;
        }

        /**
         * Maximum bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE constexpr static _STD size_t max_bucket_count() {
            return (limits<size_t>::max / 2) + 1uLL;
        }

        /**
         * Clears this  to its blank/initial state
         *
         * @author Julius
         * @date 29.10.2020
         */
        void clear() noexcept {
            _mask = 0;
        }

    private:
        _STD size_t _mask;

        static_assert(GrowthFactor >= 1uLL, "GrowthFactor have to be greater or equal to 1");
    };

    static_assert(IsGrowthPolicy<log2_growth_policy<>>);

    template <_STD size_t GrowthFactor = 2uLL, typename CalcType_ = float>
    struct sqr2_growth_policy {
        /**
         * Constructor
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  min_ The minimum.
         */
        explicit sqr2_growth_policy(const _STD size_t min_) :
            _mask(MAX(min_, 2)) {}

        /**
         * Bucket index
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  hash_ The hash.
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t bucket_idx(const _STD size_t hash_) const noexcept {
            return hash_ & _mask;
        }

        /**
         * Next bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t next_bucket_count() const {
            CalcType_ next { _STD sqrt(static_cast<CalcType_>(_mask)) };
            return _mask + static_cast<CalcType_>(next) * GrowthFactor;
        }

        /**
         * Maximum bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_t.
         */
        [[maybe_unused, nodiscard]] FORCE_INLINE constexpr static _STD size_t max_bucket_count() {
            return (limits<size_t>::max / 2) + 1uLL;
        }

        /**
         * Clears this  to its blank/initial state
         *
         * @author Julius
         * @date 29.10.2020
         */
        void clear() noexcept {
            _mask = 0;
        }

    private:
        _STD size_t _mask;

        static_assert(GrowthFactor >= 1uLL, "GrowthFactor have to be greater or equal to 1");
    };

    static_assert(IsGrowthPolicy<sqr2_growth_policy<>>);
}
