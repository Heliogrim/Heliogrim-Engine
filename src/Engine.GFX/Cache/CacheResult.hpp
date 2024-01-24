#pragma once

#include "__fwd.hpp"

namespace hg::engine::gfx::cache {
    enum CacheResult : u8 {
        eUndefined = 0x0,
        eHit = 0x1,
        //
        eMiss = 0x2
    };

    enum class StoreResultType : bool {
        eFail = false,
        eSuccess = true
    };

    enum class QueryResultType : u8 {
        eUndefined = 0x0,
        eHit = 0x1,
        eMiss = 0x2
    };

    enum class StreamCacheResultType : u8 {
        eUndefined = 0x0,
        eResidential/* eStable | eSealed | eLocked */ = 0x1,
        eTransient/* eVolatile */ = 0x2,
    };

    template <typename ResultTypeType_, typename ValueType_ = void> requires std::is_integral_v<ResultTypeType_> ||
        std::is_enum_v<ResultTypeType_>
    struct Result {
    public:
        template <typename Rx_, typename Vx_> requires std::is_integral_v<Rx_> || std::is_enum_v<Rx_>
        friend struct Result;

    public:
        using this_type = Result<ResultTypeType_, ValueType_>;

        using result_type = ResultTypeType_;
        using value_type = ValueType_;

        using store_cnd_type = std::conjunction<std::is_void<ValueType_>/*, std::is_empty<ValueType_>*/>;

        using store_type = std::conditional_t<store_cnd_type::value,
            std::type_identity_t<ResultTypeType_>,
            std::_Compressed_pair<ResultTypeType_, std::conditional_t<std::is_void_v<ValueType_>, int, ValueType_>>
        >;

    public:
        template <typename Rtt_ = ResultTypeType_, typename Vt_ = ValueType_> requires std::is_same_v<Vt_, ValueType_>
            && std::is_void_v<Vt_> && std::is_convertible_v<Rtt_, ResultTypeType_>
        constexpr Result(Rtt_&& result_) noexcept :
            data(std::forward<Rtt_>(result_)) {}

        template <
            typename Rtt_ = ResultTypeType_,
            typename Vt_ = ValueType_> requires (not std::is_void_v<Vt_>) && std::is_convertible_v<Rtt_,
                ResultTypeType_> &&
            std::is_convertible_v<Vt_, ValueType_>
        constexpr Result(Rtt_&& result_, Vt_&& value_) :
            data(std::_One_then_variadic_args_t {}, std::forward<Rtt_>(result_), std::forward<Vt_>(value_)) {}

        template <
            typename Rtt_ = ResultTypeType_,
            typename Vt_ = ValueType_> requires ((not std::is_void_v<Vt_>) && std::is_default_constructible_v<
            ValueType_>) && std::is_convertible_v<Rtt_, ResultTypeType_>
        constexpr Result(Rtt_&& result_) :
            Result<Rtt_, ValueType_>(std::forward<Rtt_>(result_), {}) {}

    public:
        store_type data;

    protected:
        template <typename Type_>
        using result_compatible = std::conjunction<
            std::is_convertible<result_type, Type_>,
            std::negation<store_cnd_type>
        >;

        template <typename Type_>
        using exclusive_result_compatible = std::conjunction<
            std::is_convertible<result_type, Type_>,
            store_cnd_type
        >;

    public:
        template <typename Type_ = ResultTypeType_> requires exclusive_result_compatible<Type_>::value
        [[nodiscard]] constexpr operator Type_() const noexcept {
            return data;
        }

        template <typename Type_ = ResultTypeType_> requires result_compatible<Type_>::value
        [[nodiscard]] constexpr operator Type_() const noexcept {
            return data._Get_first();
        }

        template <typename Type_ = ResultTypeType_> requires exclusive_result_compatible<Type_>::value
        [[nodiscard]] constexpr Type_ type() const noexcept {
            return data;
        }

        template <typename Type_ = ResultTypeType_> requires result_compatible<Type_>::value
        [[nodiscard]] constexpr Type_ type() const noexcept {
            return data._Get_first();
        }

    protected:
        template <typename Type_>
        using is_constexpr_comparator = std::conjunction<
            std::is_empty<Type_>,
            std::is_trivially_default_constructible<Type_>
        >;

        template <typename Type_>
        using constant_comparator = std::integral_constant<Type_, Type_ {}>;

    public:
        template <
            typename Type_ = ResultTypeType_,
            typename Comparator_ = std::equal_to<ResultTypeType_>> requires is_constexpr_comparator<Comparator_>::value
        [[nodiscard]] constexpr bool operator==(const auto& rhs_) const noexcept {
            return (this_type::template constant_comparator<Comparator_>::value)(
                this_type::template type<result_type>(),
                rhs_
            );
        }

        template <
            typename Type_ = ResultTypeType_,
            typename Comparator_ = std::equal_to<ResultTypeType_>> requires std::negation_v<is_constexpr_comparator<
            Comparator_>>
        [[nodiscard]] constexpr bool operator==(const auto& rhs_) const noexcept {
            return Comparator_ {}(this_type::template type<result_type>(), rhs_);
        }

        template <typename Type_ = ResultTypeType_, typename Comparator_ = std::equal_to<ResultTypeType_>>
        [[nodiscard]] constexpr bool operator!=(const auto& rhs_) const noexcept {
            return not this_type::template operator==<Type_, Comparator_>(rhs_);
        }

    protected:
        template <typename Type_>
        using supplement = std::conjunction<
            std::is_convertible<ValueType_, Type_>,
            std::negation<store_cnd_type>
        >;

        template <typename Type_>
        using exclusive_supplement = std::conjunction<
            std::negation<std::is_convertible<ResultTypeType_, Type_>>,
            std::is_convertible<ValueType_, Type_>,
            std::negation<store_cnd_type>
        >;

    public:
        template <typename Type_ = ValueType_> requires exclusive_supplement<Type_>::value
        [[nodiscard]] constexpr operator Type_() const noexcept {
            return data._Myval2;
        }

        template <typename Type_ = ValueType_> requires supplement<Type_>::value
        [[nodiscard]] constexpr Type_ value() const noexcept {
            return data._Myval2;
        }

    public:
        template <typename Type_ = ValueType_> requires supplement<Type_>::value
        [[nodiscard]] cref<Type_> operator*() const noexcept {
            return data._Myval2;
        }

        template <typename Type_ = ValueType_> requires supplement<Type_>::value
        [[nodiscard]] ref<Type_> operator*() noexcept {
            return data._Myval2;
        }

        template <typename Type_ = ValueType_> requires supplement<Type_>::value
        [[nodiscard]] const ptr<Type_> operator->() const noexcept {
            return &data._Myval2;
        }

        template <typename Type_ = ValueType_> requires supplement<Type_>::value
        [[nodiscard]] const ptr<Type_> operator->() noexcept {
            return &data._Myval2;
        }
    };
}
