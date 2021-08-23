#pragma once

#include <Engine.Common/Math/Matrix.hpp>

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

    private:
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

    private:
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

    private:
        /**
         * Look At
         */
        math::vec3 _lookAt;

    public:
        /**
         * Look at
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] const math::vec3& lookAt() const noexcept;

        /**
         * Sets look at
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  lookAt_ The look at.
         */
        void setLookAt(const math::vec3& lookAt_);

    private:
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

    private:
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

    private:
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

    public:
        /**
         * Mutations
         */

        /**
         * Updates this 
         *
         * @author Julius
         * @date 20.11.2020
         */
        void update();
    };
}
