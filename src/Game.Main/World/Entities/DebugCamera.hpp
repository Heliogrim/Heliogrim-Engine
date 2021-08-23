#pragma once
#include <Ember/World/TypedEntity.hpp>

#include "Engine.ECS.Subsystem/Reflection/EntityReflector.hpp"

using namespace ember;

namespace game::world::entity {

    class DebugCamera :
        public ember::world::TypedEntity<DebugCamera> {
    public:
        static constexpr type_id type_id = "DebugCamera"_typeId;

    public:
        void construct() noexcept;

        void assemble();

        void disassemble();

        void destruct() noexcept;
    };

    inline auto test = engine::ecs::subsystem::EntityReflector {}.operator()<DebugCamera>();
}
