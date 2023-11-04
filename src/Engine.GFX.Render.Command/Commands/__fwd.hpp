#pragma once

namespace hg::engine::gfx::render::cmd {
    class BeginRenderCommand;
    class NextSubpassRenderCommand;
    class EndRenderCommand;
    /**/
    class BindEffectRenderCommand;
    /**/
    class BindIndexBufferRenderCommand;
    class BindMaterialRenderCommand;
    class BindSkeletalMeshRenderCommand;
    class BindStaticMeshRenderCommand;
    class BindStorageBufferRenderCommand;
    class BindTextureRenderCommand;
    class BindVertexBufferRenderCommand;
    /**/
    class DrawMeshRenderCommand;
    class DrawSkeletalMeshRenderCommand;
    class DrawDispatchRenderCommand;
}

/* Forwarded Resource Declarations */

namespace hg::engine::gfx {
    class Mesh;
    class IndexBufferView;
    class VertexBufferView;
}
