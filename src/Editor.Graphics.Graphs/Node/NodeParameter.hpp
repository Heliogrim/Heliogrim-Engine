#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::graphics::graph {
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
