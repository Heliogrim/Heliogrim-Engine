#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) Visitor {
    public:
        using this_type = Visitor;

    protected:
        Visitor() noexcept = default;

    public:
        virtual ~Visitor() noexcept = default;

    public:
        virtual void operator()(cref<Node> node_) = 0;
    };
}
