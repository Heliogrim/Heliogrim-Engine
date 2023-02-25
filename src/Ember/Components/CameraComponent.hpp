#pragma once

#include <Engine.GFX/Camera/CameraModes.hpp>

#include "../ModelComponent.hpp"

namespace ember {
    class CameraComponent :
        public ModelComponent {
    public:
        using this_type = CameraComponent;
        using underlying_type = ModelComponent;

        using CameraProjectMode = ::ember::engine::gfx::CameraProjectionMode;
        using CameraViewMode = ::ember::engine::gfx::CameraViewMode;

        inline constexpr static component_type_id typeId { "CameraComponent"_typeId };

    public:
        CameraComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

        ~CameraComponent();

    private:
        float _fov;

    public:
        [[nodiscard]] float getFov() const noexcept;

        void setFov(const float fov_);

    private:
        float _aspectRation;
        bool _lockedAspect;

    public:
        [[nodiscard]] float getAspectRatio() const noexcept;

        void setAspectRatio(const float aspectRatio_);

        [[nodiscard]] bool isAspectLocked() const noexcept;

        void lockAspectRatio(const bool lock_ = true);

    private:
        bool _visibleFrustum;

    public:
        [[nodiscard]] bool isFrustumVisible() const noexcept;

        void setFrustumVisible(const bool visible_);

    private:
        math::vec3 _offset;

    public:
        [[nodiscard]] cref<math::vec3> offset() const noexcept;

        void setOffset(cref<math::vec3> offset_);

    private:
        CameraProjectMode _projectMode;
        CameraViewMode _viewMode;

    public:
        [[nodiscard]] CameraProjectMode getProjectMode() const noexcept;

        void setProjectMode(const CameraProjectMode mode_);

        [[nodiscard]] CameraViewMode getViewMode() const noexcept;

        void setViewMode(const CameraViewMode mode_);
    };
}
