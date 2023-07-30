#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Component.hpp"

#include "../Relation/Provision.hpp"
#include "../Relation/Requirement.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassComponent final :
        public InheritMeta<SubpassComponent, Component> {
    public:
        using this_type = SubpassComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassComponent() noexcept;

        ~SubpassComponent() noexcept override;

    private:
        nmpt<const Node> _mounted;
        nmpt<const SubpassAccelComponent> _accel;
        nmpt<const SubpassInvocationComponent> _invoke;

    public:
        void mount(nmpt<const Node> target_) override;

        void mounted(nmpt<const Component> mounted_) override;

        void unmount(nmpt<const Node> target_) override;

        void unmounted(nmpt<const Component> unmounted_) override;

    public:
        [[nodiscard]] DenseSet<Requirement> getRequirements() const noexcept;

        [[nodiscard]] DenseSet<Provision> getProvided() const noexcept;
    };
}
