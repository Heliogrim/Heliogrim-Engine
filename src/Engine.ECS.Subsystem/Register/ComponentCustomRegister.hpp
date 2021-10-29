#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/HopScotch.hpp>

namespace ember::engine::ecs::subsystem {
    namespace {
        struct key_value_pair {
            size_t key;
            ptr<void> value;
        };

        /**
         * Fnv-1a 86
         *
         * @author Julius
         * @date 06.09.2020
         *
         * @param  str_ The.
         * @param  count_ Number of.
         *
         * @returns A size_t.
         */
        constexpr size_t fnv1a_86(const char* str_, const _STD size_t count_) {
            return (
                (count_ ? fnv1a_86(str_, count_ - 1) : 2166136261u) ^ str_[count_]
            ) * 16777619u;
        }
    }

    struct component_custom_register {

        using value_type = component_custom_register;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using mapping_type = hopscotch_set<key_value_pair>;

    public:
        mapping_type mapping;

    public:
        [[nodiscard]] bool has(size_t key_) const noexcept {
            return mapping.contains({ key_, nullptr });
        }

        [[nodiscard]] bool has(cref<string> key_) const noexcept {
            return mapping.contains({ fnv1a_86(key_.data(), key_.length()), nullptr });
        }

        template <typename Ret_, typename ...Args_>
        struct __get_helper {
            using value_type = Ret_ (*)(Args_ ...);
        };

        template <typename Ret_, typename ...Args_>
        [[nodiscard]] typename __get_helper<Ret_, Args_...>::value_type get(cref<size_t> key_) {
            return static_cast<typename __get_helper<Ret_, Args_...>::value_type>(mapping.find({ key_, nullptr })->
                value);
        }

        template <typename Ret_, typename ...Args_>
        [[nodiscard]] typename __get_helper<Ret_, Args_...>::value_type get(cref<string> key_) {
            return static_cast<typename __get_helper<Ret_, Args_...>::value_type>(mapping.find({
                fnv1a_86(key_.data(), key_.length()),
                nullptr
            })->value);
        }

        template <typename Ret_, typename ...Args_>
        struct __set_helper {
            using value_type = Ret_ (*)(Args_ ...);
        };

        template <typename Ret_, typename ...Args_>
        void set(cref<size_t> key_, cref<typename __set_helper<Ret_, Args_...>::value_type> value_) {
            mapping.insert_or_assign({ key_, static_cast<ptr<void>>(value_) });
        }

        template <typename Ret_, typename ...Args_>
        void set(cref<string> key_, cref<typename __set_helper<Ret_, Args_...>::value_type> value_) {
            mapping.insert_or_assign({ fnv1a_86(key_.data(), key_.length()), static_cast<ptr<void>>(value_) });
        }
    };
}
