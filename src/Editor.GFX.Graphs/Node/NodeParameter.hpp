#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor::gfx::graph {
    class NodeParameter {
    public:
        using this_type = NodeParameter;

    public:
        NodeParameter(string_view uniqueName_) :
            _uniqueName(uniqueName_) {}

        virtual ~NodeParameter() noexcept = default;

    protected:
        string _uniqueName;

    public:
        [[nodiscard]] cref<string> getUniqueName() const noexcept {
            return _uniqueName;
        }
    };
}
