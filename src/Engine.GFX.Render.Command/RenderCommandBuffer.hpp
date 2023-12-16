#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

#include "RenderCommandAllocator.hpp"
#include "RenderCommandBufferBase.hpp"
#include "RenderResourceTable.hpp"

namespace hg::engine::render::cmd {
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

        void beginSubPass(mref<BeginSubPassStruct>  = {}) noexcept override;

        void nextSubPass() noexcept override;

        void endSubPass() noexcept override;

        void endAccelPass() noexcept override;

        void end() noexcept override;

    public:
        void bindComputePipeline(
            mref<smr<const accel::ComputePipeline>> pipeline_
        ) noexcept override;

        void bindGraphicsPipeline(
            mref<smr<const accel::GraphicsPipeline>> pipeline_
        ) noexcept override;

    public:
        void bindStaticMesh(
            const nmpt<const gfx::Mesh> mesh_
        ) noexcept override;

        void bindStaticMeshInstance(
            const nmpt<const gfx::MeshInstanceView> view_
        ) noexcept override;

        void bindSkeletalMesh(
            const nmpt<const gfx::Mesh> mesh_
        ) noexcept override;

        void bindSkeletalMeshInstance(
            const nmpt<const gfx::MeshInstanceView> meshView_,
            const nmpt<const gfx::SkeletalBoneView> boneView_
        ) noexcept override;

    public:
        void bindIndexBuffer(
            const nmpt<const gfx::IndexBufferView> indexView_
        ) noexcept override;

        void bindVertexBuffer(
            const nmpt<const gfx::VertexBufferView> vertexView_
        ) noexcept override;

    public:
        void bindStorage(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const gfx::StorageBufferView> storageView_
        ) noexcept override;

        void bindTexture(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const gfx::SampledTextureView> sampledTextureView_
        ) noexcept override;

        void bindTexture(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const gfx::TextureLikeObject> texture_,
            const nmpt<const gfx::TextureSampler> sampler_
        ) noexcept override;

        void bindUniform(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const gfx::UniformBufferView> uniformView_
        ) noexcept override;

        void bind(
            mref<engine::render::ResourceTable> table_
        ) noexcept override;

        void bind(
            _STD initializer_list<_STD pair<accel::lang::SymbolId, ResourceView>> list_
        ) noexcept {
            engine::render::ResourceTable tmp {};
            tmp.table.insert(list_);
            bind(_STD move(tmp));
        }

        template <typename... Types_>
        void bind(_STD pair<accel::lang::SymbolId, Types_>&&... args_) noexcept {
            engine::render::ResourceTable tmp {};

            (tmp.table.insert(
                _STD make_pair<accel::lang::SymbolId, ResourceView>(
                    _STD move(args_.first),
                    ResourceView(_STD move(args_.second))
                )
            ), ...);

            bind(_STD move(tmp));
        }

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
            const nmpt<const gfx::MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept override;

        void drawMeshIdx(
            const nmpt<const gfx::MeshDescription> meshDescription_,
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
            mref<_STD function<void(ref<accel::AccelCommandBuffer>)>> lambda_
        ) noexcept override;

    public:
        void attach(
            mref<smr<void>> obj_
        ) noexcept override;
    };
}
