#pragma once

#include "../Node.hpp"

namespace hg::editor::gfx::graph::nodes {
    class UniformInputNode :
        public Node {
    public:
        UniformInputNode() :
            Node() {}

    public:
        [[nodiscard]] bool carriesInternalDependencies() const noexcept override {
            return false;
        }

        [[nodiscard]] bool carriesGraphDependencies() const noexcept override {
            return false;
        }

        [[nodiscard]] bool carriesExternalDependencies() const noexcept override {
            return true;
        }

    public:
        [[nodiscard]] bool isAssociative() const noexcept override {
            return true;
        }

        [[nodiscard]] bool isCommutative() const noexcept override {
            return true;
        }
    };
}
