#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "CompileComponent.hpp"

#include "../../Relation/Provision.hpp"
#include "../../Relation/Requirement.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubpassComponent final :
        public InheritMeta<CompileSubpassComponent, CompileComponent> {
    public:
        using this_type = CompileSubpassComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        CompileSubpassComponent() noexcept;

        ~CompileSubpassComponent() noexcept override;

    private:
        nmpt<const Node> _mounted;
        nmpt<const CompileSubpassAccelComponent> _accel;

    public:
        void mount(nmpt<const Node> target_) override;

        void mounted(nmpt<const Component> mounted_) override;

        void unmount(nmpt<const Node> target_) override;

        void unmounted(nmpt<const Component> unmounted_) override;

    private:
        DenseSet<Requirement> _expectedRequirement;
        DenseSet<Provision> _expectedProvision;

    public:
        bool addExpectedProvision(mref<Provision> expected_);

        bool addExpectedRequirement(mref<Requirement> expected_);

    public:
        [[nodiscard, deprecated]] DenseSet<Requirement> expectedRequirement() const noexcept;

        [[nodiscard]] DenseSet<Provision> expectedProvision() const noexcept;
    };
}
