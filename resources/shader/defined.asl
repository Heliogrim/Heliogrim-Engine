#version 460 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

/******************/
/* Vertex Adopted */
/******************/

f32mat4 GetLocalToWorld();
f32mat4 GetWorldToScreen();

//fn GetLocalToWorld() -> f32mat4;
//fn GetWorldToScreen() -> f32mat4;

f32vec4 GetLocalPosition();
f32vec4 GetWorldPosition();
f32vec4 GetScreenPosition();

// fn GetLocalPosition() -> f32vec4;
// fn GetWorldPosition() -> f32vec4;
// fn GetScreenPosition() -> f32vec4;

f32vec4 GetLocalNormal();
f32vec4 GetWorldNormal();
f32vec4 GetScreenNormal();

// fn GetLocalNormal() -> f32vec4;
// fn GetWorldNormal() -> f32vec4;
// fn GetScreenNormal() -> f32vec4;

f32vec3 GetLocalUV();

// fn GetLocalUV() -> f32vec3;

/***********/
/* Defined */
/***********/

float GetFragDepth();

float GetSceneDepth(vec2 uv);

/****************/
/* User Defined */
/****************/

f32vec4 VertexWorldPosition();

f32vec4 VertexOutputPosition();