#pragma once
#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) ComplexInvocationGenerator :
        public InvocationGenerator {
    protected:
        constexpr ComplexInvocationGenerator() noexcept = default;

    public:
        constexpr ~ComplexInvocationGenerator() noexcept override = default;
    };
}
