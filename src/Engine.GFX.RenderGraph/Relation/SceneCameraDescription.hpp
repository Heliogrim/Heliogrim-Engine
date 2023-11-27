#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    class SceneCameraDescription final :
        public InheritMeta<SceneCameraDescription, Description> {
    public:
        using this_type = SceneCameraDescription;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SceneCameraDescription() noexcept;

        ~SceneCameraDescription() override;

    protected:
        [[nodiscard]] bool isValueCompatible(
            const non_owning_rptr<const Description> other_
        ) const noexcept override;
    };
}
