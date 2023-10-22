#pragma once
#include "PooledRenderCommandAllocator.hpp"
#include "RenderCommandBuffer.hpp"

namespace hg::engine::gfx::render::cmd {
    class PooledRenderCommandBuffer final :
        public RenderCommandBuffer {
    public:
        friend class RenderCommandPool;

    public:
        using this_type = PooledRenderCommandBuffer;

    public:
        PooledRenderCommandBuffer(mref<PooledRenderCommandAllocator> allocator_) noexcept :
            RenderCommandBuffer(),
            _allocator(_STD move(allocator_)) {}

        ~PooledRenderCommandBuffer() noexcept override = default;

    private:
        PooledRenderCommandAllocator _allocator;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    public:
        void begin() noexcept override;

        void nextSubpass() noexcept override;

        void end() noexcept override;

    public:
        void bindMaterial(
            mref<MaterialIdentifier> identifier_,
            const ptr<const material::Material> material_
        ) noexcept override;

        void bindStaticMesh(const ptr<const Mesh> mesh_) noexcept override;

        void bindStaticMeshInstance(const ptr<const MeshInstanceView> view_) noexcept override;

        void bindSkeletalMesh(const ptr<const Mesh> mesh_) noexcept override;

        void bindSkeletalMeshInstance(
            const ptr<const MeshInstanceView> meshView_,
            const ptr<const SkeletalBoneView> boneView_
        ) noexcept override;

    public:
        void bindIndexBuffer(const ptr<const IndexBufferView> indexView_) noexcept override;

        void bindVertexBuffer(const ptr<const VertexBufferView> vertexView_) noexcept override;

        void bindStorage(const ptr<const StorageBufferView> storageView_) noexcept override;

        void bindTexture(const ptr<const TextureView> textureView_) noexcept override;

        void bindTexture(const ptr<const VirtualTextureView> textureView_) noexcept override;

    public:
        void drawSkeletalMesh(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawSkeletalMeshIdx(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawStaticMesh(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawStaticMeshIdx(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawMesh(
            const ptr<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawMeshIdx(
            const ptr<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;
    };
}
