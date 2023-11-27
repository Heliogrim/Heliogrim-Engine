#pragma once
#include "RenderCommandAllocator.hpp"
#include "RenderCommandBufferBase.hpp"
#include "RenderResourceTable.hpp"

namespace hg::engine::gfx::render::cmd {
    class RenderCommandBuffer :
        public RenderCommandBufferBase,
        protected RenderCommandAllocator {
    public:
        using this_type = RenderCommandBuffer;

    public:
        RenderCommandBuffer() noexcept;

        RenderCommandBuffer(mref<this_type> other_) noexcept;

        RenderCommandBuffer(cref<this_type>) = delete;

        ~RenderCommandBuffer() noexcept override;

    protected:
        nmpt<RenderCommand> _begin;
        nmpt<RenderCommand> _last;

        RenderResourceTable _resourceTable;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    protected:
        [[nodiscard]] cref<RenderCommandAllocator> alloc() const noexcept;

        [[nodiscard]] ref<RenderCommandAllocator> alloc() noexcept;

    public:
        [[nodiscard]] nmpt<RenderCommand> root() const noexcept;

        void release(_Out_opt_ ptr<RenderResourceTable> resourceTable_ = nullptr);

    public:
        void begin() noexcept override;

        void beginAccelPass(mref<BeginAccelerationPassStruct>) noexcept override;

        void beginSubPass(mref<BeginSubPassStruct>) noexcept override;

        void nextSubPass() noexcept override;

        void endSubPass() noexcept override;

        void endAccelPass() noexcept override;

        void end() noexcept override;

    public:
        void bindComputePipeline(mref<smr<const acc::ComputePipeline>> pipeline_) noexcept override;

        void bindGraphicsPipeline(mref<smr<const acc::GraphicsPipeline>> pipeline_) noexcept override;

    public:
        void bindStaticMesh(const nmpt<const Mesh> mesh_) noexcept override;

        void bindStaticMeshInstance(const nmpt<const MeshInstanceView> view_) noexcept override;

        void bindSkeletalMesh(const nmpt<const Mesh> mesh_) noexcept override;

        void bindSkeletalMeshInstance(
            const nmpt<const MeshInstanceView> meshView_,
            const nmpt<const SkeletalBoneView> boneView_
        ) noexcept override;

    public:
        void bindIndexBuffer(const nmpt<const IndexBufferView> indexView_) noexcept override;

        void bindVertexBuffer(const nmpt<const VertexBufferView> vertexView_) noexcept override;

    public:
        void bindStorage(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const StorageBufferView> storageView_
        ) noexcept override;

        void bindTexture(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const TextureView> textureView_
        ) noexcept override;

        void bindTexture(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const VirtualTextureView> textureView_
        ) noexcept override;

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
            const nmpt<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawMeshIdx(
            const nmpt<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawDispatch(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 vertexCount_,
            u32 vertexOffset_
        ) noexcept override;

    public:
        void lambda(
            mref<_STD function<void(ref<AccelCommandBuffer>)>> lambda_
        ) noexcept override;
    };
}
