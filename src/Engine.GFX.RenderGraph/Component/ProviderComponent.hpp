#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Component.hpp"
#include "../Relation/Provision.hpp"

namespace hg::engine::gfx::render::graph {
    class ProviderComponent :
        public Component {
    public:
        using this_type = ProviderComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ProviderComponent() noexcept = default;

        ~ProviderComponent() noexcept override = default;

    private:
        DenseSet<Provision> _provided;

    public:
        [[nodiscard]] cref<DenseSet<Provision>> getProvided() const noexcept;

        void setProvided(mref<DenseSet<Provision>> provided_);
    };
}
