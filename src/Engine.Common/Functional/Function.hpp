#pragma once

#include <functional>

#include "../Wrapper.hpp"

namespace ember {
    /**
     * Template build for Nular-Function-Pointer
     */
    template <typename Return = void>
    using nular_fnc = Return (*)();

    /**
     * Template build for Unary-Function-Pointer
     */
    template <typename Return, typename Param>
    using unary_fnc = Return (*)(Param);

    /**
     * Template build for Binary-Function-Pointer
     */
    template <typename Return, typename First, typename Second>
    using binary_fnc = Return (*)(First, Second);

    /**
     * Template build for Tuple-Function-Pointer
     */
    template <typename Return, typename... Params>
    using tuple_fnc = Return (*)(Params...);

    /**
     * Template build for Bound-Tuple-Function-Pointer
     */
    template <typename Bound, typename Return, typename... Params>
    using bound_tuple_fnc = Return (Bound::*)(Params...);

    template <typename Bound_, typename Return_, typename... Params_>
    using const_bound_tuple_fnc = Return_ (Bound_::*)(Params_...) const;

    /**
     * Forward Declaration
     *
     * @author Julius
     * @date 21.08.2021
     *
     * @tparam Type_ Type of the type.
     */
    template <typename Type_>
    struct function_helper;

    /**
     * Forward Declaration
     *
     * @author Julius
     * @date 21.08.2021
     *
     * @tparam Type_ Type of the type.
     */
    template <typename Type_>
    struct function_helper_ne;

    template <typename Type_, typename Return_, typename... Args_>
    struct function_helper<bound_tuple_fnc<Type_, Return_, Args_...>> {
        template <Return_(Type_::*Fnc)(Args_...)>
        static Return_ launch_pad(ref<_STD remove_cvref_t<Type_>> self_, Args_&&... args_) {
            return (self_.*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...)>
        static Return_ unsafe_launch_pad(ptr<void> self_, Args_&&... args_) {
            return (static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...)>
        static void unsafe_void_launch_pad(ptr<void> self_, Args_&&... args_) {
            (void)(static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }
    };

    template <typename Type_, typename Return_, typename... Args_>
    struct function_helper_ne<Return_ (Type_::*)(Args_...) noexcept> {
        template <Return_(Type_::*Fnc)(Args_...) noexcept>
        static Return_ launch_pad(ref<_STD remove_cvref_t<Type_>> self_, Args_&&... args_) noexcept {
            return (self_.*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) noexcept>
        static Return_ unsafe_launch_pad(ptr<void> self_, Args_&&... args_) noexcept {
            return (static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) noexcept>
        static void unsafe_void_launch_pad(ptr<void> self_, Args_&&... args_) noexcept {
            (void)(static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }
    };

    template <typename Type_, typename Return_, typename... Args_>
    struct function_helper<bound_tuple_fnc<Type_, Return_, Args_...> const> {
        template <Return_(Type_::*Fnc)(Args_...) const>
        static Return_ launch_pad(cref<_STD remove_cvref_t<Type_>> self_, Args_&&... args_) {
            return (self_.*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) const>
        static Return_ unsafe_launch_pad(ptr<void> self_, Args_&&... args_) {
            return (static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) const>
        static void unsafe_void_launch_pad(ptr<void> self_, Args_&&... args_) {
            (void)(static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }
    };

    template <typename Type_, typename Return_, typename... Args_>
    struct function_helper_ne<Return_ (Type_::*)(Args_...) const noexcept> {
        template <Return_(Type_::*Fnc)(Args_...) const noexcept>
        static Return_ launch_pad(cref<_STD remove_cvref_t<Type_>> self_, Args_&&... args_) noexcept {
            return (self_.*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) const noexcept>
        static Return_ unsafe_launch_pad(ptr<void> self_, Args_&&... args_) noexcept {
            return (static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }

        template <Return_(Type_::*Fnc)(Args_...) const noexcept>
        static void unsafe_void_launch_pad(ptr<void> self_, Args_&&... args_) noexcept {
            (void)(static_cast<Type_*>(self_)->*Fnc)(_STD forward<Args_>(args_)...);
        }
    };
}
