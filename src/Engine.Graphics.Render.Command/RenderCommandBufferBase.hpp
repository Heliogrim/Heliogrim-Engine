#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Graphics.RenderGraph/Relation/MeshDescription.hpp>

#include "Commands/Structs.hpp"
#include "Resource/Pipeline.hpp"
#include "Resource/ResourceTable.hpp"

namespace hg::engine::accel {
    class AccelCommandBuffer;
}

namespace hg::engine::graphics {
    class Mesh;
    class MeshInstanceView;
    class SkeletalBoneView;
    class IndexBufferView;
    class VertexBufferView;
    class StorageBufferView;
    class UniformBufferView;
    class SparseTextureView;
    struct MaterialIdentifier;
    using MeshDescription = ::hg::engine::render::graph::MeshDescription;
    class TextureSampler;
}

namespace hg::engine::graphics::material {
    class Material;
}

namespace hg::engine::render::cmd {
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

        virtual void beginSubPass(mref<BeginSubPassStruct>  = {}) noexcept = 0;

        virtual void nextSubPass() noexcept = 0;

        virtual void endSubPass() noexcept = 0;

        virtual void endAccelPass() noexcept = 0;

        virtual void end() noexcept = 0;

    public:
        virtual void bindComputePipeline(mref<smr<const accel::ComputePipeline>> pipeline_) noexcept = 0;

        virtual void bindGraphicsPipeline(mref<smr<const accel::GraphicsPipeline>> pipeline_) noexcept = 0;

    public:
        virtual void bindStaticMesh(const nmpt<const graphics::Mesh> mesh_) noexcept = 0;

        virtual void bindStaticMeshInstance(const nmpt<const graphics::MeshInstanceView> view_) noexcept = 0;

        virtual void bindSkeletalMesh(const nmpt<const graphics::Mesh> mesh_) noexcept = 0;

        virtual void bindSkeletalMeshInstance(
            const nmpt<const graphics::MeshInstanceView> meshView_,
            const nmpt<const graphics::SkeletalBoneView> boneView_
        ) noexcept = 0;

    public:
        virtual void bindIndexBuffer(const nmpt<const graphics::IndexBufferView> indexView_) noexcept = 0;

        virtual void bindVertexBuffer(const nmpt<const graphics::VertexBufferView> vertexView_) noexcept = 0;

    public:
        virtual void bindStorage(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const graphics::StorageBufferView> storageView_
        ) noexcept = 0;

        virtual void bindTexture(
            mref<accel::lang::SymbolId> symbolId_,
            mref<nmpt<const graphics::TextureLikeObject>> texture_
        ) noexcept = 0;

        virtual void bindTextureSampler(
            mref<accel::lang::SymbolId> symbolId_,
            mref<nmpt<const graphics::TextureSampler>> sampler_
        ) noexcept = 0;

        virtual void bindUniform(
            mref<accel::lang::SymbolId> symbolId_,
            const nmpt<const graphics::UniformBufferView> storageView_
        ) noexcept = 0;

        virtual void bind(
            mref<smr<engine::render::ResourceTable>> table_
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
            const nmpt<const graphics::MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = graphics::MeshDescription> requires
            std::derived_from<MeshDescType_, graphics::MeshDescription> &&
            std::is_nothrow_convertible_v<MeshDescType_, graphics::MeshDescription>
        void drawMesh(
            const nmpt<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMesh(
                static_cast<const nmpt<const graphics::MeshDescription>>(meshDescription_),
                instanceCount_,
                instanceOffset_,
                primitiveCount_,
                primitiveOffset_
            );
        }

        virtual void drawMeshIdx(
            const nmpt<const graphics::MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = graphics::MeshDescription> requires
            std::derived_from<MeshDescType_, graphics::MeshDescription> &&
            std::is_nothrow_convertible_v<MeshDescType_, graphics::MeshDescription>
        void drawMeshIdx(
            const nmpt<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMeshIdx(
                static_cast<const nmpt<const graphics::MeshDescription>>(meshDescription_),
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
            mref<std::function<void(ref<accel::AccelCommandBuffer>)>> lambda_
        ) noexcept = 0;

    public:
        virtual void attach(
            mref<smr<void>> obj_
        ) noexcept = 0;
    };
}
