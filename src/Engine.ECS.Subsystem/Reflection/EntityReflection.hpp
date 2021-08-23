#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember::engine::ecs::subsystem {

    struct EntityReflection {
    public:
        /** Identifier for the type */
        type_id typeId = 0ui64;

    public:
        /** Size of the memory */
        size_t memSize;

    public:
        using constructor_fnc_type = void (*)(ptr<void> this_) noexcept;
        constructor_fnc_type constructor;

        using assemble_fnc_type = void (*)(ptr<void> this_);
        assemble_fnc_type assemble;

        using disassemble_fnc_type = void (*)(ptr<void> this_);
        disassemble_fnc_type disassemble;

        using destructor_fnc_type = void (*)(ptr<void> this_) noexcept;
        destructor_fnc_type destructor;
    };

}
