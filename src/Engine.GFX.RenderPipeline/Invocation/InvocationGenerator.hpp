#pragma once

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) InvocationGenerator {
    protected:
        InvocationGenerator() noexcept = default;

    public:
        virtual ~InvocationGenerator() noexcept = default;
    };
}
