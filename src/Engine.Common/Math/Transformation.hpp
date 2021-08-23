#pragma once

#include "../stdafx.h"
#include "Quaternion.hpp"

namespace ember::math {
    class Transformation {
    public:
        using type = Transformation;

        using value_type = Transformation;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        /**
         * \brief 
         * \param position_ 
         * \param rotation_ 
         * \param scale_ 
         */
        Transformation(const vec3& position_, const vec3& rotation_, const vec3& scale_) :
            _rotation(rotation_),
            _quaternion(0),
            _position(position_),
            _scale(scale_) { }

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 03.08.2021
         *
         * @param  other_ The other.
         */
        Transformation(const_reference_type other_) :
            _rotation(other_._rotation),
            _matrix(other_._matrix),
            _quaternion(other_._quaternion),
            _position(other_._position),
            _scale(other_._scale) {}

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 03.08.2021
         *
         * @param  other_ The other.
         */
        Transformation(mref<Transformation> other_) noexcept :
            _rotation(_STD exchange(other_._rotation, {})),
            _matrix(_STD exchange(other_._matrix, {})),
            _quaternion(_STD exchange(other_._quaternion, {})),
            _position(_STD exchange(other_._position, {})),
            _scale(_STD exchange(other_._scale, {})) {}

    public:
        /**
         * Mutations
         */
        void resolveMatrix() {
            /**
             * Reset Matrix
             */
            _matrix.identity();

            /**
             * Resolve Values ( Reverse Order )
             */
            _matrix.rotate(glm::radians(_rotation.x), vec3(1.F, 0.F, 0.F))
                   .rotate(glm::radians(_rotation.y), vec3(0.F, 1.F, 0.F))
                   .rotate(glm::radians(_rotation.z), vec3(0.F, 0.F, 1.F))
                   .translate(_position)
                   .scale(_scale.x);
        }

    public:
        /**
         * Getters & Setters
         */

        /**
         * \brief 
         * \return 
         */
        const vec3& position() const {
            return _position;
        }

        /**
         * \brief 
         * \return 
         */
        vec3& position() {
            return _position;
        }

        /**
         * \brief 
         * \return 
         */
        const vec3& rotation() const {
            return _rotation;
        }

        /**
         * \brief 
         * \return 
         */
        vec3& rotation() {
            return _rotation;
        }

        /**
         * \brief
         * \return 
         */
        [[nodiscard]] quaternion::const_reference_type quaternion() const {
            return _quaternion;
        }

        /**
         * \brief 
         * \return 
         */
        quaternion::reference_type quaternion() {
            return _quaternion;
        }

        /**
         * \brief 
         * \return 
         */
        const vec3& scale() const {
            return _scale;
        }

        /**
         * \brief 
         * \return 
         */
        vec3& scale() {
            return _scale;
        }

        /**
         * \brief 
         * \return 
         */
        const mat4& matrix() const {
            return _matrix;
        }

        /**
         * \brief 
         * \return 
         */
        mat4& matrix() {
            return _matrix;
        }

        /**
         * \brief 
         */
        operator const mat4&() const {
            return _matrix;
        }

        /**
         * \brief 
         */
        operator mat4&() {
            return _matrix;
        }

        /**
         * \brief 
         * \param other_ 
         * \return 
         */
        reference_type operator =(const reference_type other_) {
            _rotation = other_._rotation;
            _matrix = other_._matrix;
            _quaternion = other_._quaternion;
            _position = other_._position;
            _scale = other_._scale;

            return *this;
        }

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 03.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<Transformation> other_) noexcept {
            if (this != &other_) {
                _rotation = _STD exchange(other_._rotation, {});
                _matrix = _STD exchange(other_._matrix, {});
                _quaternion = _STD exchange(other_._quaternion, {});
                _position = _STD exchange(other_._position, {});
                _scale = _STD exchange(other_._scale, {});
            }

            return *this;
        }

        // reference_type conjunct(const reference_type other_) {
        // 	return *this;
        // }
        //
        // type conjuncted(const reference_type other_) const {
        // 	return {};
        // }

    private:
        vec3 _rotation;

        mat4 _matrix;

        /**
         * 
         */
        quaternion::type _quaternion;
        vec3 _position;
        vec3 _scale;
    };

    /**
     * \brief 
     * \param left_ 
     * \param right_ 
     * \return 
     */
    inline mat4 operator *(const mat4& left_, const Transformation& right_) {
        return left_ * right_.matrix();
    }

    class ZeroTransformation :
        public Transformation {
    public:
        /**
         * \brief 
         */
        ZeroTransformation() :
            Transformation(vec3(), vec3(), vec3(1.F)) { }
    };
}
