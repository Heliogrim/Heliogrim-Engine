﻿#pragma once

#include <type_traits>
#include "Types.hpp"

namespace ember {

    template <typename Ty, class UTy = u8>
    struct Flag {
        static_assert(_STD is_enum<Ty>::value, "Enumeration required.");

        /** Type of the value */
        using value_type = UTy;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.12.2020
         */
        Flag() noexcept = default;

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         */
        Flag(const Flag& other_) noexcept = default;

        /**
         * Move constructor
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param [in,out] other_ The other.
         */
        Flag(Flag&& other_) noexcept = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  initial_ The initial.
         */
        Flag(const Ty& initial_) noexcept :
            unwrap(static_cast<value_type>(initial_)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.12.2020
         */
        ~Flag() noexcept = default;

        /**
         * Bitwise 'or' assignment operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        Flag& operator |=(const Flag& other_) {
            unwrap |= other_.unwrap;
            return *this;
        }

        /**
         * Bitwise 'or' assignment operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        Flag& operator |=(const Ty& other_) {
            unwrap |= static_cast<value_type>(other_);
            return *this;
        }

        /**
         * Bitwise 'or' operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        Flag operator|(const Flag& other_) const noexcept {
            Flag res { *this };
            return res |= other_;
        }

        /**
         * Bitwise 'or' operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        Flag operator|(const Ty& other_) const noexcept {
            Flag res { *this };
            return res |= other_;
        }

        /**
         * Bitwise 'and' operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        bool operator&(const Flag& other_) {
            return unwrap & other_.unwrap;
        }

        /**
         * Bitwise 'and' operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns The result of the operation.
         */
        bool operator&(const Ty& other_) const noexcept {
            return unwrap & static_cast<value_type>(other_);
        }

        /**
         * Assignment operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        Flag operator=(const Ty& other_) const noexcept {
            return Flag { other_ };
        }

        /**
         * Assignment operator
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        Flag& operator=(const Ty& other_) noexcept {
            unwrap = static_cast<value_type>(other_);
            return *this;
        }

        /** The unwrap */
        value_type unwrap;
    };
}

template <typename Ty, class UTy>
ember::Flag<Ty, UTy> operator|(const Ty& left_, const Ty& right_) {
    return ember::Flag<Ty, UTy> { left_ } |= right_;
}
