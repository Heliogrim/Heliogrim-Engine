#pragma once

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) InvocationGenerator {
    protected:
        constexpr InvocationGenerator() noexcept = default;

    public:
        constexpr virtual ~InvocationGenerator() noexcept = default;
    };
}
