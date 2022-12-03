#pragma once

#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Quaternion.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "CameraModes.hpp"

namespace ember::engine::gfx {

    class Camera {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        Camera() = default;

    protected:
        CameraViewMode _mode;

    public:
        [[nodiscard]] CameraViewMode viewMode() const noexcept;

        void setViewMode(const CameraViewMode mode_);

    private:
        /**
         * Aspect Ratio
         */
        float _aspect = 16.F / 9.F;

    public:
        /**
         * Gets the aspect ratio
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float aspect() const noexcept;

        /**
         * Sets the aspect ratio
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  ratio_ The ratio.
         */
        void setAspect(const float ratio_);

    protected:
        /**
         * FOV
         */
        float _fov = 70.F;

    public:
        /**
         * Gets the fov
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float fov() const noexcept;

        /**
         * Sets the fov
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  fov_ The fov.
         */
        void setFov(const float fov_);

    protected:
        /**
         * Clipping Planes
         */
        float _znear = 0.001F;
        float _zfar = 1000.F;

    public:
        /**
         * Near clip
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float nearClip() const noexcept;

        /**
         * Far clip
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float farClip() const noexcept;

        /**
         * Gets the projection matrix
         *
         * @author Julius
         * @date 20.11.2020
         */
        [[nodiscard]] math::mat4 projection() const noexcept;

    protected:
        /**
         * Position
         */
        math::vec3 _position;

    public:
        /**
         * Gets the position
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] const math::vec3& position() const noexcept;

        /**
         * Sets a position
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  position_ The position.
         */
        void setPosition(const math::vec3& position_);

    protected:
        math::quaternion _rotation;

    public:
        [[nodiscard]] cref<math::quaternion> rotation() const noexcept;

        void setRotation(cref<math::quaternion> rotation_);

        void setRotation(cref<math::vec3> euler_);

    protected:
        /**
         * Perspective Matrix
         */
        math::mat4 _perspective;

    public:
        /**
         * Sets a perspective
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  fov_ The fov.
         * @param  aspectRatio_ The aspect ratio.
         * @param  near_ The near.
         * @param  far_ The far.
         */
        void setPerspective(const float fov_, const float aspectRatio_, const float near_, const float far_);

    protected:
        /**
         * View Matrix
         */
        math::mat4 _view;

    public:
        /**
         * Gets the view matrix
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A const.
         */
        const math::mat4& view() const noexcept;

    private:
        void updateView();

    public:
        /**
         *
         */
        void lookAt(cref<math::vec3> target_);
    };
}
