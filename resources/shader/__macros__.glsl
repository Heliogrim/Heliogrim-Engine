// #version 460 core

#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_ARB_separate_shader_objects : enable

/**/
/* == Depth Passes ==*/
/**/

#if defined(HG_SCTX_DEPTH_ONLY) && defined(HG_SCTX_TYPE_VERTEX)

    #define HG_VERT_FWD_BIND_POSITION 0
    #define HG_VERT_FWD_BIND_NORMAL 1
    #define HG_VERT_FWD_BIND_UV 2

    layout (location = HG_VERT_FWD_BIND_POSITION) in vec3 VertexPosition;
    layout (location = HG_VERT_FWD_BIND_NORMAL) in vec3 VertexNormal;
    layout (location = HG_VERT_FWD_BIND_UV) in vec3 VertexUv;

    layout (set = 0, binding = 0) uniform TVertexCamera {
        f32mat4 vp;
    } VertexCamera;

    #if defined(HG_SCTX_SUB_INSTANCED)
        layout (set = 1, binding = 0) uniform TVertexInstance {
            f32mat4 m;
        } VertexInstance;
    #else
        struct TVertexInstance {
            f32mat4 m;
        };

        const TVertexInstance VertexInstance = TVertexInstance(
            f32mat4(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            )
        );
    #endif

    f32mat4 GetLocalToWorld() {
        return VertexInstance.m;
    }

    f32mat4 GetLocalToScreen() {
        return VertexCamera.vp * VertexInstance.m;
    }

    f32mat4 GetWorldToScreen() {
        return VertexCamera.vp;
    }

    f32vec4 GetLocalPosition() {
        return f32vec4(VertexPosition, 1.0f);
    }

    f32vec4 GetWorldPosition() {
        return GetLocalToWorld() * GetLocalPosition();
    }

    f32vec4 GetScreenPosition() {
        return VertexCamera.vp * VertexInstance.m * GetLocalPosition();
    }

    f32vec4 GetLocalNormal() {
        return f32vec4(VertexNormal, 0.0f);
    }

    f32vec4 GetWorldNormal() {
        return VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
    }

    f32vec4 GetScreenNormal() {
        return VertexCamera.vp * VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
    }

    f32vec3 GetLocalUV() {
        return VertexUv;
    }

    f32vec4 VertexWorldPosition() {
        return GetWorldPosition();
    }

    f32vec4 VertexOutputPosition() {
        return GetWorldToScreen() * VertexWorldPosition();
    }

#endif

#if defined(HG_SCTX_DEPTH_ONLY) && defined(HG_SCTX_TYPE_FRAGMENT)
#endif

/**/
/* == Material Passes ==*/
/**/

#if defined(HG_SCTX_MATERIAL_PASS) && defined(HG_SCTX_TYPE_VERTEX)

    #define HG_VERT_FWD_BIND_POSITION 0
    #define HG_VERT_FWD_BIND_NORMAL 1
    #define HG_VERT_FWD_BIND_UV 2

    #define VS_TO_FRAG_WORLD_POSITION 0
    #define VS_TO_FRAG_LOCAL_NORMAL 1
    #define VS_TO_FRAG_LOCAL_UV 2

    layout (location = HG_VERT_FWD_BIND_POSITION) in vec3 VertexPosition;
    layout (location = HG_VERT_FWD_BIND_NORMAL) in vec3 VertexNormal;
    layout (location = HG_VERT_FWD_BIND_UV) in vec3 VertexUv;

    layout (location = VS_TO_FRAG_WORLD_POSITION) out f32vec4 outWorldPosition;
    layout (location = VS_TO_FRAG_LOCAL_NORMAL) out f32vec4 outLocalNormal;
    layout (location = VS_TO_FRAG_LOCAL_UV) out f32vec3 outLocalUv;

    layout (set = 0, binding = 0) uniform TVertexCamera {
        f32mat4 vp;
    } VertexCamera;

    #if defined(HG_SCTX_SUB_INSTANCED)
        layout (set = 1, binding = 0) uniform TVertexInstance {
            f32mat4 m;
        } VertexInstance;
    #else
        struct TVertexInstance {
            f32mat4 m;
        };

        const TVertexInstance VertexInstance = TVertexInstance(
            f32mat4(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            )
        );
    #endif

    f32mat4 GetLocalToWorld() {
        return VertexInstance.m;
    }

    f32mat4 GetLocalToScreen() {
        return VertexCamera.vp * VertexInstance.m;
    }

    f32mat4 GetWorldToScreen() {
        return VertexCamera.vp;
    }

    f32vec4 GetLocalPosition() {
        return f32vec4(VertexPosition, 1.0f);
    }

    f32vec4 GetWorldPosition() {
        return GetLocalToWorld() * GetLocalPosition();
    }

    f32vec4 GetScreenPosition() {
        return VertexCamera.vp * VertexInstance.m * GetLocalPosition();
    }

    f32vec4 GetLocalNormal() {
        return f32vec4(VertexNormal, 0.0f);
    }

    f32vec4 GetWorldNormal() {
        return VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
    }

    f32vec4 GetScreenNormal() {
        return VertexCamera.vp * VertexInstance.m * f32vec4(normalize(GetLocalNormal()).xyz, 0.0f);
    }

    f32vec3 GetLocalUV() {
        return VertexUv;
    }

    f32vec4 VertexWorldPosition() {
        return GetWorldPosition();
    }

    f32vec4 VertexOutputPosition() {
        return GetWorldToScreen() * VertexWorldPosition();
    }

#endif

#if defined(HG_SCTX_MATERIAL_PASS) && defined(HG_SCTX_TYPE_FRAGMENT)

    #define VS_TO_FRAG_WORLD_POSITION 0
    #define VS_TO_FRAG_LOCAL_NORMAL 1
    #define VS_TO_FRAG_LOCAL_UV 2

    layout (location = VS_TO_FRAG_WORLD_POSITION) in f32vec4 inWorldPosition;
    layout (location = VS_TO_FRAG_LOCAL_NORMAL) in f32vec4 inLocalNormal;
    layout (location = VS_TO_FRAG_LOCAL_UV) in f32vec3 inLocalUv;

    layout (set = 0, binding = 0) uniform TFragmentCamera {
        f32mat4 vp;
    } FragmentCamera;

#endif