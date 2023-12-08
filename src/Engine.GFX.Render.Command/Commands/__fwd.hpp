#pragma once

namespace hg::engine::render::cmd {
    class RenderCommand;
    /**/
    class BeginRCmd;
    class BeginAccelerationPassRCmd;
    class BeginSubPassRCmd;
    class NextSubpassRCmd;
    class EndSubPassRCmd;
    class EndAccelerationPassRCmd;
    class EndRCmd;
    /**/
    class BindPipelineRCmd;
    class BindResourceTableRCmd;
    /**/
    class BindIndexBufferRCmd;
    class BindSkeletalMeshRCmd;
    class BindStaticMeshRCmd;
    class BindStorageBufferRCmd;
    class BindTextureRCmd;
    class BindUniformBufferRCmd;
    class BindVertexBufferRCmd;
    /**/
    class DrawMeshRCmd;
    class DrawSkeletalMeshRCmd;
    class DrawDispatchRCmd;
    class DrawDispatchIndirectRCmd;
    /**/
    class LambdaRCmd;
    /**/
    class AttachResourceRCmd;
}

/* Forwarded Resource Declarations */

namespace hg::engine::gfx {
    class Mesh;
    class IndexBufferView;
    class VertexBufferView;
}
