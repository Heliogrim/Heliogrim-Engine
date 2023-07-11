#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render::pipeline {
    enum class ExecStateFlag : u8 {
        eUndefined = 0x0,
        eRunning = 0x1,
        eDone = 0x2
    };

    struct ExecState {
        nmpt<_STD atomic_uint_fast8_t> cnd;
        _STD atomic_uint_fast8_t state;

    public:
        [[nodiscard]] bool running() const noexcept {
            return static_cast<ExecStateFlag>(state.load(_STD memory_order::acquire)) == ExecStateFlag::eRunning;
        }

        [[nodiscard]] bool done() const noexcept {
            return static_cast<ExecStateFlag>(state.load(_STD memory_order::acquire)) == ExecStateFlag::eDone;
        }
    };
}
