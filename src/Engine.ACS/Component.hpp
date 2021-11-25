#pragma once
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember::engine::acs {

    template <typename Ty>
    concept IsComponent = ember::HasStaticType<Ty> &&
    _STD is_object_v<Ty> &&
    _STD is_nothrow_default_constructible_v<Ty>;
    /* This will require the default constructor for a object to be noexcept attributed *///&&
    //_STD is_trivially_move_assignable_v<Ty> &&
    //_STD is_trivially_move_constructible_v<Ty>;

    /**
     * A component handle which will be force by type cast of component reference using undefined behaviour to control pooled references.
     *
     * @author Julius
     * @date 31.08.2020
     *
     * @tparam ComponentType Type of the component type.
     */
    template <IsComponent ComponentType>
    struct component_handle {
    public:
        using internal_type = ComponentType;
        using internal_ref_type = ComponentType&;
        using internal_ptr_type = ComponentType*;

        using value_type = component_handle<internal_type>;
        using reference_type = component_handle<internal_type>&;
        using const_reference_type = const component_handle<internal_type>&;

        /**
         * The pointer the handle is created above
         */
        const internal_ptr_type ref;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.09.2020
         */
        component_handle() = delete;

        /**
         * Constructor
         * 
         *	Will create a invalid component_handle
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param nullptr
         */
        component_handle(nullptr_t) :
            ref(nullptr) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param ref_ The reference to a valid component.
         */
        component_handle(const internal_ref_type ref_) :
            ref(&ref_) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param ptr_ The pointer to a valid component.
         */
        component_handle(const internal_ptr_type& ptr_) :
            ref(ptr_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param  other_ The other.
         */
        component_handle(const_reference_type other_) :
            ref(other_.ref) {}

        /**
         * Pseudo Move Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param [in,out] other_ The other.
         */
        component_handle(value_type&& other_) :
            ref(_STD move(other_.ref)) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @tparam OtherType Type of the other type.
         * @param  other_ The other.
         */
        template <IsComponent OtherType>
        component_handle(typename component_handle<OtherType>::const_reference_type other_) :
            ref(other_.ref) {}

        /**
         * Pseudo Move Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @tparam OtherType Type of the other type.
         * @param  other_ The other.
         */
        template <IsComponent OtherType>
        component_handle(typename component_handle<OtherType>::value_type&& other_) :
            ref(_STD move(other_.ref)) {}

        /**
         * Check whether this is valid pointer
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const {
            return ref != nullptr;
        }

        /**
         * Indirection operator
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A immutable reference to the stored component; If this is invalid, function has undefined behavior.
         */
        const ComponentType& operator *() const {
            // TODO: Check whether we want reinterpret_cast or static_cast
            return *(reinterpret_cast<const ComponentType*>(ref));
        }

        /**
         * Indirection operator
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns A mutable reference to the stored component; If this is invalid, function has undefined behavior.
         */
        ComponentType& operator *() {
            return *(reinterpret_cast<ComponentType*>(ref));
        }

        /**
         * Member dereference operator
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns The dereferenced object.
         */
        const ComponentType* operator->() const {
            return (reinterpret_cast<ComponentType*>(ref));
        }

        /**
         * Member dereference operator
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns The dereferenced object.
         */
        ComponentType* operator->() {
            return (reinterpret_cast<ComponentType*>(ref));
        }

        /**
         * Const ComponentType* casting operator
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns The result of the operation.
         */
        operator const ComponentType*() const {
            return (reinterpret_cast<const ComponentType*>(ref));
        }

        /**
         * ComponentType* casting operator
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns The result of the operation.
         */
        operator ComponentType*() {
            return (reinterpret_cast<ComponentType*>(ref));
        }

        /**
         * Query if this is null
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns True if null, false if not.
         */
        [[nodiscard]] bool is_null() const noexcept {
            return ref == nullptr;
        }

        /**
         * Check whether this is valid pointer
         *
         * @author Julius
         * @date 31.08.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool valid() const noexcept {
            return ref != nullptr;
        }
    };

    /**
     * A null component.
     *
     * @author Julius
     * @date 31.08.2020
     */
    struct null_component_t {};

    /** The null component */
    inline constexpr null_component_t null_component {};

    /**
     * Equality operator
     *
     * @author Julius
     * @date 31.08.2020
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    template <IsComponent ComponentType>
    bool operator ==(const typename component_handle<ComponentType>::reference_type left_, null_component_t right_) {
        return left_.is_null();
    }

    /**
     * Equality operator
     *
     * @author Julius
     * @date 31.08.2020
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    template <IsComponent ComponentType>
    bool operator ==(null_component_t left_, const typename component_handle<ComponentType>::reference_type right_) {
        return right_.is_null();
    }
}
