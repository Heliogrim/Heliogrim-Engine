#pragma once

#include <Engine.Common/Functional/Function.hpp>

#include "EntityReflection.hpp"

namespace ember::engine::ecs::subsystem {

    struct EntityReflector {
        template <typename Ty>
        [[nodiscard]] EntityReflection operator()() noexcept {

            constexpr auto constructor_callback = &function_helper_ne<decltype(&Ty::construct)>
                ::template unsafe_launch_pad<&Ty::construct>;
            constexpr auto assemble_callback = &function_helper<decltype(&Ty::assemble)>
                ::template unsafe_launch_pad<&Ty::assemble>;
            constexpr auto disassemble_callback = &function_helper<decltype(&Ty::disassemble)>
                ::template unsafe_launch_pad<&Ty::disassemble>;
            constexpr auto destructor_callback = &function_helper_ne<decltype(&Ty::destruct)>
                ::template unsafe_launch_pad<&Ty::destruct>;

            return {
                Ty::type_id,
                sizeof(Ty),
                constructor_callback,
                assemble_callback,
                disassemble_callback,
                destructor_callback
            };
        }
    };

}
