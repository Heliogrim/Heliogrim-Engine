#pragma once

#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::graphics::graph {
    class NodeLink {
    public:
        using this_type = NodeLink;

    private:
        wptr<NodeOutput> _linkInput;

    public:
        void setLinkInput(mref<wptr<NodeOutput>> linkInput_);

        [[nodiscard]] wptr<NodeOutput> getLinkInput();

    private:
        CompactArray<wptr<NodeInput>> _linkOutputs;

    public:
        void addLinkOutput(mref<wptr<NodeInput>> linkOutput_);

        void removeLinkOutput(cref<sptr<NodeInput>> linkOutput_);

        [[nodiscard]] bool hasLinkOutput() const noexcept;

        [[nodiscard]] size_t countLinkOutputs() const noexcept;

    public:
        [[nodiscard]] NodeLinkTransportMode getTransportMode() const noexcept;
    };
}
