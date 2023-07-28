#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) StateAware {
    public:
        using this_type = StateAware;

    protected:
        StateAware() noexcept = default;

    public:
        virtual ~StateAware() noexcept = default;

    public:
        /**
         * Used as callback to register typed and index sub-state infos.
         *
         * @param state_ The state where to register and later allocate.
         */
        virtual void regSubState(_Inout_ nmpt<State> state_) const;

    public:
        [[nodiscard]] virtual bool isReallocSupported() const noexcept;

        virtual bool alloc(_Inout_ nmpt<State> state_) = 0;

        virtual bool realloc(_Inout_ nmpt<State> state_);

        virtual bool free(_Inout_ nmpt<State> state_) = 0;
    };
}
