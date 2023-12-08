#pragma once
#include <Engine.Common/Math/Hash.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Camera/CameraFrustum.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

#include "Culling.hpp"

namespace hg::engine::render::graph {
    class FrustumCullingFilter :
        public InheritMeta<FrustumCullingFilter, CullingFilter> {
    public:
        using this_type = FrustumCullingFilter;

    public:
        constexpr FrustumCullingFilter() noexcept = default;

        constexpr ~FrustumCullingFilter() noexcept override = default;

    private:
        gfx::CameraFrustum _frustum;
        ptr<const gfx::scene::SceneView> _sceneView;

    public:
        [[nodiscard]] checksum_type checksum() const noexcept override {

            constexpr auto selfTypeId = reflect::typeId<this_type>();
            checksum_type combined { selfTypeId.data };

            const auto sceneHash = reinterpret_cast<checksum_type>(_sceneView->getScene());

            // TODO: Hash view- and projection-matrix
            const auto viewHash = _sceneView->getViewMatrix();
            const auto projHash = _sceneView->getProjectionMatrix();

            hash::hashCombine<checksum_type>(combined, sceneHash);

            return combined;
        }

    public:
        void update(cref<gfx::scene::SceneView> sceneView_) noexcept override;

        [[nodiscard]] bool operator()(scene_node_type node_) const noexcept override;
    };
}
