#pragma once
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.ECS/Entity.hpp>

namespace ember::engine::ecs::subsystem {

    struct ComponentReflection {
    public:
        /** Identifier for the type */
        type_id typeId = 0ui64;

    public:
        /** Size of the memory */
        size_t memSize = 0ui64;

    public:
        /**
         * Type of the constructor function
         * 
         * @see Component.hpp [Creating]
         */
        using constructor_fnc_type = void (*)();
        constructor_fnc_type constructor = nullptr;

        /**
         * Type of the materialize function
         *
         *  @see Component.hpp [Materialize]
         */
        using materialize_fnc_type = void (*)(Entity::const_reference_type);
        materialize_fnc_type materialize = nullptr;

        /**
         * Type of the dematerialize function
         *
         * @see Component.hpp [Dematerialize]
         */
        using dematerialize_fnc_type = void (*)(Entity::const_reference_type);
        dematerialize_fnc_type dematerialize = nullptr;

        /**
         * Type of the destructor function
         *
         * @see Component.hpp [Destroy]
         */
        using destructor_fnc_type = void (*)();
        destructor_fnc_type destructor = nullptr;

    public:
    };

}
