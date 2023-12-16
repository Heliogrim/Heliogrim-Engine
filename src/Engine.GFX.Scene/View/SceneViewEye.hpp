#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Location.hpp>
#include <Engine.Common/Math/Rotator.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Quaternion.hpp>

namespace hg::engine::gfx::scene {
    class SceneViewEye {
    public:
        using this_type = SceneViewEye;

    public:
        SceneViewEye();

    protected:
        math::Location _location;

        math::mat4 _projection;
        math::mat4 _view;

    protected:
        math::Location _vkLocation;
        bool _vkFlipY;

    public:
        void updateProjection(cref<math::mat4> projection_);

        void updateView(cref<math::Location> origin_, cref<math::Rotator> rotation_);

        void updateView(cref<math::Location> origin_, cref<math::quaternion> rotation_);

    public:
        [[nodiscard]] cref<math::Location> getOrigin() const noexcept;

        [[nodiscard]] cref<math::mat4> getProjectionMatrix() const noexcept;

        void getInvProjectionMatrix(_Inout_ ref<math::mat4> matrix_) const noexcept;

        [[nodiscard]] cref<math::mat4> getViewMatrix() const noexcept;

        void getInvViewMatrix(_Inout_ ref<math::mat4> matrix_) const noexcept;

    public:
        [[nodiscard]] math::vec4 worldToView(const math::vec4 worldPosition_) const;

        [[nodiscard]] math::vec4 viewToWorld(const math::vec4 viewPosition_) const;

    public:
        /**
         * Calculate the projected world space position into ui space
         *
         * @author Julius
         * @date 03.12.2022
         *
         * @param worldPosition_ The world space position to project into ui space
         *
         * @returns A vector in normalized space `[0.0 ... 1.0]` when  succeeded, otherwise {-1, -1}
         */
        [[nodiscard]] math::vec2 worldToScreen(cref<math::vec4> worldPosition_) const;

        /**
         * Calculate the trace origin and direction un-projected from ui position to world space
         *
         * @author Julius
         * @date 03.12.2022
         *
         * @param screenPosition_ The ui space position in normalized space `[0.0 ... 1.0]`
         * @param [Out] worldOrigin_ The world space origin position for the trace
         * @param [Out] worldDirection_ The world space vector direction for the trace
         */
        void screenToWorld(
            cref<math::vec2> screenPosition_,
            _Out_ ref<math::vec3> worldOrigin_,
            _Out_ ref<math::vec3> worldDirection_
        ) const;

    public:
        [[deprecated]] void vkSetFlipY(bool flip_);

        [[nodiscard]] cref<math::Location> getVkLocation() const noexcept;
    };
}
