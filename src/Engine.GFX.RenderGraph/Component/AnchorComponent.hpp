#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class AnchorComponent final :
        public Component {
    public:
        using this_type = AnchorComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        AnchorComponent() noexcept;

        AnchorComponent(mref<string> anchorName_) noexcept;

        ~AnchorComponent() noexcept override = default;

    private:
        string _name;

    public:
        [[nodiscard]] string_view getAnchorName() const noexcept;
    };
}
