#pragma once

#include <variant>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "../NodeLinkValueType.hpp"

namespace hg::editor::gfx::graph {
    template <NodeLinkValueType ValueType_>
    struct ConstantNodeParameterTypeStorage;

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::u8> {
        using type = u8;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::u16> {
        using type = u16;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::u32> {
        using type = u32;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::u64> {
        using type = u64;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32> {
        using type = float;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f64> {
        using type = double;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec2> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::vec2_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec3> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::vec3_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec4> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::vec4_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat2x2> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::matq2_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat3x3> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::matq3_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat4x4> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::matq4_t<sub_type>;
        type value;
    };

    template <>
    struct ConstantNodeParameterTypeStorage<NodeLinkValueType::color> {
        using sub_type = ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type;
        using type = math::vec4_t<sub_type>;
        type value;
    };

    /**/

    typedef _STD variant<
        /**/
        ConstantNodeParameterTypeStorage<NodeLinkValueType::u8>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::u16>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::u32>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::u64>::type,
        /**/
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f64>::type,
        /**/
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec2>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec3>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32vec4>::type,
        /**/
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat2x2>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat3x3>::type,
        ConstantNodeParameterTypeStorage<NodeLinkValueType::f32mat4x4>::type
        /**/
        //ConstantNodeParameterTypeStorage<NodeLinkValueType::color>::type // Does already exists as `f32vec4`
    > ConstantNodeParameterStorage;
}
