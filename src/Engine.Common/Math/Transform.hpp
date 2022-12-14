#pragma once

#include "Quaternion.hpp"
#include "Vector.hpp"
#include "../Wrapper.hpp"

namespace ember::math {
    class Transform {
    public:
        using this_type = Transform;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 26.11.2021
         */
        constexpr Transform() noexcept :
            _position(0.F),
            _rotation(quaternion::make_identity()),
            _scale(1.F) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 26.11.2021
         *
         * @param position_ The position as vec3.
         * @param rotation_ The rotation as quaternion.
         * @param scale_ The scale as vec3.
         */
        Transform(cref<vec3> position_, cref<quaternion> rotation_, cref<vec3> scale_) noexcept :
            _position(position_),
            _rotation(rotation_),
            _scale(scale_) {}

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 26.11.2021
         *
         * @param other_ The transform to move from.
         */
        Transform(mref<Transform> other_) noexcept = default;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 26.11.2021
         *
         * @param other_ The transform to copy from.
         */
        Transform(cref<Transform> other_) = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 26.11.2021
         */
        ~Transform() = default;

    public:
        /**
         * Move Assignment
         *
         * @author Julius
         * @date 02.12.2021
         *
         * @param other_ The transform to move from.
         *
         * @returns A shallow copy of this.
         */
        ref<Transform> operator=(mref<Transform> other_) noexcept;

        /**
         * Copy Assignment
         *
         * @author Julius
         * @date 02.12.2021
         *
         * @param other_ The transform to copy from.
         *
         * @returns A shallow copy of this.
         */
        ref<Transform> operator=(cref<Transform> other_) = default;

    private:
        vec3 _position;

    public:
        [[nodiscard]] cref<vec3> position() const noexcept {
            return _position;
        }

        void setPosition(cref<vec3> position_) noexcept {
            _position = position_;
        }

        void setPosition(mref<vec3> position_) noexcept {
            _position = position_;
        }

    private:
        quaternion _rotation;

    public:
        [[nodiscard]] cref<quaternion> rotation() const noexcept {
            return _rotation;
        }

        void setRotation(cref<quaternion> rotation_) noexcept {
            _rotation = rotation_;
        }

        void setRotation(mref<quaternion> rotation_) noexcept {
            _rotation = rotation_;
        }

    private:
        vec3 _scale;

    public:
        [[nodiscard]] cref<vec3> scale() const noexcept {
            return _scale;
        }

        void setScale(cref<vec3> scale_) noexcept {
            _scale = scale_;
        }

        void setScale(mref<vec3> scale_) noexcept {
            _scale = scale_;
        }
    };
}
