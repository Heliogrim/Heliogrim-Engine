#version 460 core

#include <defined>

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

@input (mode = forward)
vec3 VertexPosition;

@input (mode = forward)
vec3 VertexNormal;

@input (mode = forward)
vec3 VertexUV;

@input (mode = bind, id = "camera")
struct VertexCamera {
    f32mat4 vp;
};

@input (mode = bind, id = "instance")
struct VertexInstance {
    f32mat4 m;
};

/**/

f32mat4 GetLocalToWorld() {
    return VertexInstance.m;
}

f32mat4 GetLocalToScreen() {
    return VertexCamera.vp * VertexInstance.m;
}

f32mat4 GetWorldToScreen() {
    return VertexCamera.vp;
}

//fn GetLocalToWorld() -> f32mat4;
//fn GetWorldToScreen() -> f32mat4;

f32vec4 GetLocalPosition() {
    return f32vec4(VertexPosition, 1.0f);
}

f32vec4 GetWorldPosition() {
    return GetLocalToWorld() * GetLocalPosition();
}

f32vec4 GetScreenPosition() {
    return VertexCamera.vp * VertexCamera.m + GetLocalPosition();
}

// fn GetLocalPosition() -> f32vec4;
// fn GetWorldPosition() -> f32vec4;
// fn GetScreenPosition() -> f32vec4;

f32vec4 GetLocalNormal() {
    return VertexNormal;
}

f32vec4 GetWorldNormal() {
    return VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
}

f32vec4 GetScreenNormal() {
    return VertexCamera.vp * VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
}

// fn GetLocalNormal() -> f32vec4;
// fn GetWorldNormal() -> f32vec4;
// fn GetScreenNormal() -> f32vec4;

f32vec3 GetLocalUV() {
    return VertexUV;
}

// fn GetLocalUV() -> f32vec3;

/****************/
/* User Defined */
/****************/

@redefine (allow = true)
f32vec4 VertexWorldPosition() {
    return GetWorldPosition();
}

@redefine (allow = true)
f32vec4 VertexOutputPosition() {
    return GetWorldToScreen() * VertexWorldPosition();
}