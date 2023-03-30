#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace hg::editor::gfx::graph {
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
