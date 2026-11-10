#pragma once

#include "../Node.hpp"

namespace hg::editor::gfx::graph::nodes {
    class TextureInputNode :
        public Node {
    public:
        TextureInputNode() :
            Node() {}

    public:
        [[nodiscard]] bool carriesInternalDependencies() const noexcept override {
            return false;
        }

        [[nodiscard]] bool carriesGraphDependencies() const noexcept override {
            return true/* Maybe */;
        }

        [[nodiscard]] bool carriesExternalDependencies() const noexcept override {
            return true;
        }

    public:
        [[nodiscard]] bool isAssociative() const noexcept override {
            return true;
        }

        [[nodiscard]] bool isCommutative() const noexcept override {
            return false/* Maybe */;
        }
    };
}
