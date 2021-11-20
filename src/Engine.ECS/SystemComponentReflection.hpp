#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Component.hpp"
#include "ComponentTypeId.hpp"
#include "Traits.hpp"

namespace ember::engine::ecs {

    struct SystemComponentReflectionDummy {};

    template <typename ComponentType_>
    concept IsMaterializeableComponent = IsComponent<ComponentType_> && requires(ComponentType_ obj,
    cref<entity_guid> entity_) {
        { obj.materialize(entity_) } -> std::same_as<void>;
    };

    template <typename ComponentType_>
    concept IsDematerializeableComponent = IsComponent<ComponentType_> && requires(ComponentType_ obj,
    cref<entity_guid> entity_) {
        { obj.dematerialize(entity_) } -> std::same_as<void>;
    };

    struct SystemComponentReflection {
    public:
        /** Identifier for the type */
        component_type_id typeId { 0ui64 };

    public:
        /** Size of the memory */
        size_t memSize = 0ui64;

    public:
        /**
         * Type of the constructor function
         * 
         * @see Component.hpp [Creating]
         */
        using constructor_fnc_type = void (SystemComponentReflectionDummy::*)();
        constructor_fnc_type constructor = nullptr;

        /**
         * Type of the materialize function
         *
         *  @see Component.hpp [Materialize]
         */
        using materialize_fnc_type = void (SystemComponentReflectionDummy::*)(_In_ cref<entity_guid> entity_);
        materialize_fnc_type materialize = nullptr;

        /**
         * Type of the dematerialize function
         *
         * @see Component.hpp [Dematerialize]
         */
        using dematerialize_fnc_type = void (SystemComponentReflectionDummy::*)(_In_ cref<entity_guid> entity_);
        dematerialize_fnc_type dematerialize = nullptr;

        /**
         * Type of the destructor function
         *
         * @see Component.hpp [Destroy]
         */
        using destructor_fnc_type = void (SystemComponentReflectionDummy::*)();
        destructor_fnc_type destructor = nullptr;

    public:
    };
}
