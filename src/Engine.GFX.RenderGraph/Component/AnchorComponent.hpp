#pragma once
#include <Engine.Common/String.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class AnchorComponent final :
        public Component {
    public:
        using this_type = AnchorComponent;

    public:
        AnchorComponent() noexcept = default;

        AnchorComponent(mref<string> anchorName_) noexcept;

        ~AnchorComponent() noexcept override = default;

    private:
        string _name;

    public:
        [[nodiscard]] string_view getAnchorName() const noexcept;
    };
}
