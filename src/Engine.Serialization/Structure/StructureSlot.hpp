#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "StructurePath.hpp"
#include "__fwd.hpp"

#include "StructureSlotType.hpp"

namespace ember::engine::serialization {
    class __declspec(novtable) StructureSlotBase {
    public:
        using this_type = StructureSlotBase;

    protected:
        StructureSlotBase() = default;

    public:
        virtual ~StructureSlotBase() = default;

    public:
        [[nodiscard]] virtual StructureSlotType getSlotType() const noexcept = 0;

    protected:
        virtual void enter(const bool mutating_) = 0;

        virtual void leave(const bool mutating_) = 0;

    public:
        /*
        virtual void enumerateSlots(
            _Inout_ ref<Vector<const non_owning_rptr<ScopedStructureSlotBase>>> enum_
        ) const noexcept;
         */
    };
}
