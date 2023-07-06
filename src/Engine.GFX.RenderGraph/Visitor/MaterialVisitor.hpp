#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.GFX.Material/__fwd.hpp>

#include "Visitor.hpp"

namespace hg::engine::gfx::render::graph {
    class MaterialVisitor :
        public Visitor {
    public:
        using this_type = MaterialVisitor;

    public:
        MaterialVisitor() = default;

        ~MaterialVisitor() noexcept override = default;

    public:
        void operator()(cref<Node> node_) override;

    private:
        DenseSet<smr<material::Material>> _orphaned;
        DenseSet<smr<material::Material>> _raised;

    public:
        [[nodiscard]] cref<DenseSet<smr<material::Material>>> getOrphanedMaterials() const noexcept;

        [[nodiscard]] cref<DenseSet<smr<material::Material>>> getRaisedMaterials() const noexcept;

    public:
        void addOrphanedMaterial(mref<smr<material::Material>> orphaned_);

        void addRaisedMaterial(mref<smr<material::Material>> raised_);
    };
}
