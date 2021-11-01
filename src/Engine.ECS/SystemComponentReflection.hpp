#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Component.hpp"
#include "ComponentTypeId.hpp"
#include "Traits.hpp"

namespace ember::engine::ecs {

    struct SystemComponentReflectionDummy {};

    template <typename ComponentType_>
    concept IsMantleableComponent = IsComponent<ComponentType_> && requires(ComponentType_ obj,
    cref<entity_guid> entity_) {
        { obj.mantle(entity_) } -> std::same_as<void>;
    };

    template <typename ComponentType_>
    concept IsDismantleableComponent = IsComponent<ComponentType_> && requires(ComponentType_ obj,
    cref<entity_guid> entity_) {
        { obj.dismantle(entity_) } -> std::same_as<void>;
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
         * Type of the mantle function
         *
         *  @see Component.hpp [Mantle]
         */
        using mantle_fnc_type = void (SystemComponentReflectionDummy::*)(_In_ cref<entity_guid> entity_);
        mantle_fnc_type mantle = nullptr;

        /**
         * Type of the dismantle function
         *
         * @see Component.hpp [Dismantle]
         */
        using dismantle_fnc_type = void (SystemComponentReflectionDummy::*)(_In_ cref<entity_guid> entity_);
        dismantle_fnc_type dismantle = nullptr;

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
