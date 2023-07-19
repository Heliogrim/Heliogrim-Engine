#pragma once

#include "MetaClass.hpp"

namespace hg {
    class ClassMetaBase;
}

namespace hg {
    namespace {
        template <typename...>
        struct detail_filter_list;

        template <typename, typename>
        struct detail_filter_list_append;

        template <typename... Lhs_, typename... Rhs_>
        struct detail_filter_list_append<detail_filter_list<Lhs_...>, detail_filter_list<Rhs_...>> {
            using type = detail_filter_list<Lhs_..., Rhs_...>;
        };

        template <typename... Args_>
        struct detail_filter_list_op;

        template <>
        struct detail_filter_list_op<> {
            using type = detail_filter_list<>;
        };

        template <typename Arg_, typename... Rest_>
        struct detail_filter_list_op<Arg_, Rest_...> {
            using recursion = detail_filter_list_op<Rest_...>;
            using subject = _STD conditional_t<
                _STD derived_from<Arg_, ClassMetaBase>,
                detail_filter_list<Arg_>,
                detail_filter_list<>
            >;

            using type = typename detail_filter_list_append<subject, typename recursion::type>::type;
        };

        template <typename... InheritType_>
        using detail_filter_list_t = typename detail_filter_list_op<InheritType_...>::type;
    }

    /**/

    template <typename TargetType_>
    class TypedMetaClass final :
        public MetaClass {
    public:
        template <typename>
        friend class InheritBase;

        template <typename, typename, typename...>
        friend class InheritMeta;

    public:
        using this_type = TypedMetaClass<TargetType_>;
        using target_type = TargetType_;

    private:
        static inline _STD atomic<ptr<this_type>> _instance = nullptr;

    private:
        TypedMetaClass() :
            MetaClass() {}

        ~TypedMetaClass() = default;

    public:
        [[nodiscard]] static const __restricted_ptr<const this_type> get() noexcept {
            return _instance.load(_STD memory_order::relaxed);
        }

        /**
         * @warning Unsafe method when used in a multi-execution context
         */
        static void destroy() {
            this_type* next = nullptr;
            auto* prev = _instance.exchange(next, _STD memory_order::acq_rel);
            delete prev;
        }

    protected:
        [[nodiscard]] static const __restricted_ptr<const this_type> getOrCreate();

        [[nodiscard]] static const __restricted_ptr<const this_type> inherit(
            const __restricted_ptr<const MetaClass> other_
        );

        template <typename... InheritType_> requires (sizeof...(InheritType_) > 0)
        [[nodiscard]] static const __restricted_ptr<const this_type> inherit();
    };

    /**/

    template <typename>
    struct lookup;

    template <typename... InheritType_>
    struct lookup<detail_filter_list<InheritType_...>> {
        [[nodiscard]] constexpr Vector<ptr<const MetaClass>> operator()() noexcept {
            return Vector<ptr<const MetaClass>> {
                (TypedMetaClass<InheritType_>::get())...
            };
        }
    };

    /**/

    template <typename TargetType_>
    const __restricted_ptr<const typename TypedMetaClass<TargetType_>::this_type> TypedMetaClass<
        TargetType_>::getOrCreate() {

        auto* stored = _instance.load(_STD memory_order::relaxed);
        if (stored != nullptr) [[likely]]
        {
            return stored;
        }

        // Fallback
        {
            auto* next = new this_type();

            // TODO: Inject values
            next->_typeId = force_constexpr<ctid<TargetType_>()>;

            this_type* expect = nullptr;
            if (not _instance.compare_exchange_strong(expect, next, _STD memory_order::acq_rel)) {
                delete next;
            }
        }

        return _instance.load(_STD memory_order::consume);
    }

    template <typename TargetType_>
    const __restricted_ptr<const typename TypedMetaClass<TargetType_>::this_type> TypedMetaClass<TargetType_>::inherit(
        const __restricted_ptr<const MetaClass> other_
    ) {

        auto* stored = _instance.load(_STD memory_order::relaxed);
        if (stored != nullptr) [[likely]]
        {
            return stored;
        }

        // Fallback
        {
            auto* next = new this_type();

            // TODO: Inject values
            next->_typeId = force_constexpr<ctid<TargetType_>()>;

            if (other_ != nullptr) {
                next->_inherit = {
                    { other_->typeId() },
                    { other_ },
                };
            }

            /**/

            this_type* expect = nullptr;
            if (not _instance.compare_exchange_strong(expect, next, _STD memory_order::acq_rel)) {
                delete next;
            }
        }

        return _instance.load(_STD memory_order::consume);
    }

    template <typename TargetType_>
    template <typename... InheritType_> requires (sizeof...(InheritType_) > 0)
    const __restricted_ptr<const typename TypedMetaClass<TargetType_>::this_type> TypedMetaClass<
        TargetType_>::inherit() {

        auto* stored = _instance.load(_STD memory_order::relaxed);
        if (stored != nullptr) [[likely]]
        {
            return stored;
        }

        // Fallback
        {
            auto* next = new this_type();

            // TODO: Inject values
            next->_typeId = force_constexpr<ctid<TargetType_>()>;

            using details = detail_filter_list_t<InheritType_...>;
            next->_inherit = {
                { force_constexpr<ctid<InheritType_>()>... },
                lookup<details> {}()
            };

            this_type* expect = nullptr;
            if (not _instance.compare_exchange_strong(expect, next, _STD memory_order::acq_rel)) {
                delete next;
            }
        }

        return _instance.load(_STD memory_order::consume);
    }
}
