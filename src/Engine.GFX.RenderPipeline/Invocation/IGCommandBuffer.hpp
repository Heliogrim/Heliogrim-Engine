#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render::pipeline {
    class IGCommandBuffer {
    public:
        void draw(
            u32 instanceCount_,
            u32 vertexCount_
        );

        void drawIndexed(
            u32 instanceCount_,
            u32 indexCount_
        );

        /**/

        void bindMaterial(
            mref<const struct MaterialIdentifier> identifier_,
            ptr<const class Material> material_
        );

        void bindStaticMesh(
            ptr<const class Mesh> mesh_
        );

        void bindStaticInstances(
            ptr<const class MeshInstanceView> view_
        );

        void bindSkeletalMesh(
            ptr<const class Mesh> mesh_
        );

        void bindSkeletalInstances(
            ptr<const class MeshInstanceView> view_,
            ptr<const class SkeletalBoneView> boneView_
        );

        /**/

        void bindVertexBuffer(
            ptr<const class VertexBufferView> vertexView_
        );

        void bindIndexBuffer(
            ptr<const class IndexBufferView> indexView_
        );

        void bindTexture(
            ptr<const class VirtualTextureView> textureView_
        );

        void bindStorage(
            ptr<const class StorageBufferView> storageView_
        );
    };
}
