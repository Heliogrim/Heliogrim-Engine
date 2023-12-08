#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Description.hpp"

namespace hg::engine::render::graph {
    class SceneViewDescription final :
        public InheritMeta<SceneViewDescription, Description> {
    public:
        using this_type = SceneViewDescription;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SceneViewDescription() noexcept;

        ~SceneViewDescription() override;

    protected:
        [[nodiscard]] bool isValueCompatible(
            const non_owning_rptr<const Description> other_
        ) const noexcept override;
    };
}
