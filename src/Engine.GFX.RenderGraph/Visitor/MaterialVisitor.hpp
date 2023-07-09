#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
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

    public:
        void operator()(cref<SubpassNode> node_) override;

    private:
        DenseSet<smr<MaterialResource>> _orphaned;
        DenseSet<smr<MaterialResource>> _raised;

    public:
        [[nodiscard]] cref<DenseSet<smr<MaterialResource>>> getOrphanedMaterials() const noexcept;

        [[nodiscard]] cref<DenseSet<smr<MaterialResource>>> getRaisedMaterials() const noexcept;

    public:
        void addOrphanedMaterial(mref<smr<MaterialResource>> orphaned_);

        void addRaisedMaterial(mref<smr<MaterialResource>> raised_);

    private:
        nmpt<cache::GlobalCacheCtrl> _globalCache;

        size_t _currentTracking;
        Vector<DenseSet<smr<acc::AccelerationEffect>>> _tracking;

    protected:
        [[nodiscard]] size_t getCurrentTracking() const noexcept;

        [[nodiscard]] smr<acc::AccelerationEffect> selectProgressiveEffect(
            size_t trackingIndex_,
            const ptr<const material::Material> material_
        ) const noexcept;
    };
}
