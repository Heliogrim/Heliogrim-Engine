#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    class ModelDescription final :
        public Description {
    public:
        using this_type = ModelDescription;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ModelDescription() noexcept;

        ~ModelDescription() override;

    protected:
        [[nodiscard]] bool isValueCompatible(
            const non_owning_rptr<const Description> other_
        ) const noexcept override;
    };
}
