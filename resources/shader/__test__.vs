//#version 450 core

#extension GL_EXT_shader_explicit_arithmetic_types : enable

#extension GL_ARB_separate_shader_objects : enable

#define HG_NAMED_MESH_TYPE_STATIC 1
#define HG_NAMED_MESH_TYPE_SKELETAL 2

#define VS_TO_FRAG_WORLD_POSITION 5701
#define VS_TO_FRAG_LOCAL_NORMAL 5702
#define VS_TO_FRAG_LOCAL_UV 5703

#if defined(HG_SCTX_DEPTH_ONLY)
    /*  */

    #if defined(HG_NAMED_MESH_TYPE) && HG_NAMED_MESH_TYPE == HG_NAMED_MESH_TYPE_STATIC
        /*  */
        /*  */
        void main() {
            gl_Position = VertexOutputPosition();
        }
        /*  */
    #endif

    #if defined(HG_NAMED_MESH_TYPE) && HG_NAMED_MESH_TYPE == HG_NAMED_MESH_TYPE_SKELETAL
        /*  */
        /*  */
        void main() {
            gl_Position = VertexOutputPosition();
        }
        /*  */
    #endif

    /*  */
#endif

/**/
/**/
/**/

#if defined(HG_SCTX_MATERIAL_PASS)
    /*  */

    #if defined(HG_NAMED_MESH_TYPE) && HG_NAMED_MESH_TYPE == HG_NAMED_MESH_TYPE_STATIC
        /*  */
        layout (location = VS_TO_FRAG_WORLD_POSITION) out f32vec4 outWorldPosition;
        layout (location = VS_TO_FRAG_LOCAL_NORMAL) out f32vec4 outLocalNormal;
        layout (location = VS_TO_FRAG_LOCAL_UV) out f32vec3 outLocalUv;
        /*  */
        void main() {
            gl_Position = VertexOutputPosition();

            outWorldPosition = VertexWorldPosition();
            outLocalNormal = GetLocalNormal();
            outLocalUv = GetLocalUV();
        }
        /*  */
    #endif

    #if defined(HG_NAMED_MESH_TYPE) && HG_NAMED_MESH_TYPE == HG_NAMED_MESH_TYPE_SKELETAL
        /*  */
        layout (location = VS_TO_FRAG_WORLD_POSITION) out f32vec4 outWorldPosition;
        layout (location = VS_TO_FRAG_LOCAL_NORMAL) out f32vec4 outLocalNormal;
        layout (location = VS_TO_FRAG_LOCAL_UV) out f32vec3 outLocalUv;
        /*  */
        void main() {
            gl_Position = VertexOutputPosition();

            outWorldPosition = VertexWorldPosition();
            outLocalNormal = GetLocalNormal();
            outLocalUv = GetLocalUV();
        }
        /*  */
    #endif

    /*  */
#endif