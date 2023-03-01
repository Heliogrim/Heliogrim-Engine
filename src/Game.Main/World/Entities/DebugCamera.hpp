#pragma once
#include <Heliogrim/World/TypedEntity.hpp>
#include <Engine.ECS.Subsystem/Reflection/EntityReflector.hpp>

using namespace hg;

namespace game::world::entity {

    class DebugCamera :
        public ::hg::world::TypedEntity<DebugCamera> {
    public:
        static constexpr type_id typeId = "DebugCamera"_typeId;

    public:
        void construct() noexcept;

        void assemble();

        void disassemble();

        void destruct() noexcept;
    };

    inline auto test = engine::acs::subsystem::EntityReflector {}.operator()<DebugCamera>();
}
