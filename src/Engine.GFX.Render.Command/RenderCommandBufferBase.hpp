#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

#include "Commands/Structs.hpp"
#include "Resource/Pipeline.hpp"

namespace hg::engine::gfx {
    class Mesh;
    class MeshInstanceView;
    class SkeletalBoneView;
    class IndexBufferView;
    class VertexBufferView;
    class StorageBufferView;
    class TextureView;
    class VirtualTextureView;
    struct MaterialIdentifier;
    using MeshDescription = ::hg::engine::gfx::render::graph::MeshDescription;
    class AccelCommandBuffer;
}

namespace hg::engine::gfx::acc {
    class AccelerationEffect;
}

namespace hg::engine::gfx::material {
    class Material;
}

namespace hg::engine::gfx::render::cmd {
    class RenderCommandBufferBase {
    public:
        using this_type = RenderCommandBufferBase;

    protected:
        constexpr RenderCommandBufferBase() noexcept = default;

    public:
        constexpr virtual ~RenderCommandBufferBase() noexcept = default;

    public:
        virtual void begin() noexcept = 0;

        virtual void beginAccelPass(mref<BeginAccelerationPassStruct>) noexcept = 0;

        virtual void beginSubPass(mref<BeginSubPassStruct>) noexcept = 0;

        virtual void nextSubPass() noexcept = 0;

        virtual void endSubPass() noexcept = 0;

        virtual void endAccelPass() noexcept = 0;

        virtual void end() noexcept = 0;

    public:
        virtual void bindComputePipeline(mref<smr<const acc::ComputePipeline>> pipeline_) noexcept = 0;

        virtual void bindGraphicsPipeline(mref<smr<const acc::GraphicsPipeline>> pipeline_) noexcept = 0;

    public:
        virtual void bindStaticMesh(const nmpt<const Mesh> mesh_) noexcept = 0;

        virtual void bindStaticMeshInstance(const nmpt<const MeshInstanceView> view_) noexcept = 0;

        virtual void bindSkeletalMesh(const nmpt<const Mesh> mesh_) noexcept = 0;

        virtual void bindSkeletalMeshInstance(
            const nmpt<const MeshInstanceView> meshView_,
            const nmpt<const SkeletalBoneView> boneView_
        ) noexcept = 0;

    public:
        virtual void bindIndexBuffer(const nmpt<const IndexBufferView> indexView_) noexcept = 0;

        virtual void bindVertexBuffer(const nmpt<const VertexBufferView> vertexView_) noexcept = 0;

    public:
        virtual void bindStorage(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const StorageBufferView> storageView_
        ) noexcept = 0;

        virtual void bindTexture(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const TextureView> textureView_
        ) noexcept = 0;

        virtual void bindTexture(
            const nmpt<const acc::Symbol> symbol_,
            const nmpt<const VirtualTextureView> textureView_
        ) noexcept = 0;

    public:
        virtual void drawSkeletalMesh(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        virtual void drawSkeletalMeshIdx(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        virtual void drawStaticMesh(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        virtual void drawStaticMeshIdx(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        virtual void drawMesh(
            const nmpt<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = MeshDescription> requires
            _STD derived_from<MeshDescType_, MeshDescription> &&
            _STD is_nothrow_convertible_v<MeshDescType_, MeshDescription>
        void drawMesh(
            const nmpt<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMesh(
                static_cast<const nmpt<const MeshDescription>>(meshDescription_),
                instanceCount_,
                instanceOffset_,
                primitiveCount_,
                primitiveOffset_
            );
        }

        virtual void drawMeshIdx(
            const nmpt<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = MeshDescription> requires
            _STD derived_from<MeshDescType_, MeshDescription> &&
            _STD is_nothrow_convertible_v<MeshDescType_, MeshDescription>
        void drawMeshIdx(
            const nmpt<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMeshIdx(
                static_cast<const nmpt<const MeshDescription>>(meshDescription_),
                instanceCount_,
                instanceOffset_,
                primitiveCount_,
                primitiveOffset_
            );
        }

        virtual void drawDispatch(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 vertexCount_,
            u32 vertexOffset_
        ) noexcept = 0;

    public:
        virtual void lambda(
            mref<_STD function<void(ref<AccelCommandBuffer>)>> lambda_
        ) noexcept = 0;
    };
}
