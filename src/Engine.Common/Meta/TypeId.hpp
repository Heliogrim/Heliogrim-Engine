#pragma once

#include <concepts>

namespace ember {

    /**
     * A class type identifier.
     *
     * @author Julius
     * @date 06.09.2020
     */
    typedef size_t type_id;

    namespace {
        /**
         * Fnv-1a 86
         *
         * @author Julius
         * @date 06.09.2020
         *
         * @param  str_ The.
         * @param  count_ Number of.
         *
         * @returns A type_id.
         */
        constexpr type_id fnv1a_86(const char* str_, const _STD size_t count_) {
            return (
                (count_ ? fnv1a_86(str_, count_ - 1) : 2166136261u) ^ str_[count_]
            ) * 16777619u;
        }
    }

    /**
     * Cast that converts the given const char* to a ""_typeId
     *
     * @author Julius
     * @date 06.09.2020
     *
     * @param  str_ The.
     * @param  count_ Number of.
     *
     * @returns The result of the operation.
     */
    constexpr type_id operator"" _typeId(const char* str_, const _STD size_t count_) {
        return fnv1a_86(str_, count_);
    }

    /**
     * Type has static type identifier
     * 
     */
    template <typename Ty>
    concept HasStaticType = requires(Ty) {
        { Ty::type_id };
    };

    /**
     * Type has dynamic type identifier
     * 
     */
    template <typename Ty>
    concept HasDynamicType = requires(Ty obj) {
        { obj.get_typeId() } -> std::same_as<type_id>;
    };

    /**
     * Type has static and a dynamic type identifier
     * 
     */
    template <typename Ty>
    concept HasType = HasStaticType<Ty> && HasDynamicType<Ty>;
}
