#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

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
}

namespace hg::engine::gfx::acc {
    class AccelerationEffect;
}

namespace hg::engine::gfx::material {
    class Material;
}

namespace hg::engine::gfx::render::cmd {
    class RenderCommandBuffer {
    public:
        using this_type = RenderCommandBuffer;

    protected:
        constexpr RenderCommandBuffer() noexcept = default;

    public:
        constexpr virtual ~RenderCommandBuffer() noexcept = default;

    public:
        virtual void begin() noexcept = 0;

        virtual void nextSubpass() noexcept = 0;

        virtual void end() noexcept = 0;

    public:
        virtual void bindEffect(const ptr<const acc::AccelerationEffect> effect_) noexcept = 0;

    public:
        virtual void bindMaterial(
            mref<MaterialIdentifier> identifier_,
            const ptr<const material::Material> material_
        ) noexcept = 0;

        virtual void bindStaticMesh(const ptr<const Mesh> mesh_) noexcept = 0;

        virtual void bindStaticMeshInstance(const ptr<const MeshInstanceView> view_) noexcept = 0;

        virtual void bindSkeletalMesh(const ptr<const Mesh> mesh_) noexcept = 0;

        virtual void bindSkeletalMeshInstance(
            const ptr<const MeshInstanceView> meshView_,
            const ptr<const SkeletalBoneView> boneView_
        ) noexcept = 0;

    public:
        virtual void bindIndexBuffer(const ptr<const IndexBufferView> indexView_) noexcept = 0;

        virtual void bindVertexBuffer(const ptr<const VertexBufferView> vertexView_) noexcept = 0;

        virtual void bindStorage(const ptr<const StorageBufferView> storageView_) noexcept = 0;

        virtual void bindTexture(const ptr<const TextureView> textureView_) noexcept = 0;

        virtual void bindTexture(const ptr<const VirtualTextureView> textureView_) noexcept = 0;

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
            const ptr<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = MeshDescription> requires
            _STD derived_from<MeshDescType_, MeshDescription> &&
            _STD is_nothrow_convertible_v<MeshDescType_, MeshDescription>
        void drawMesh(
            const ptr<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMesh(
                static_cast<const ptr<const MeshDescription>>(meshDescription_),
                instanceCount_,
                instanceOffset_,
                primitiveCount_,
                primitiveOffset_
            );
        }

        virtual void drawMeshIdx(
            const ptr<const MeshDescription> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;

        template <typename MeshDescType_ = MeshDescription> requires
            _STD derived_from<MeshDescType_, MeshDescription> &&
            _STD is_nothrow_convertible_v<MeshDescType_, MeshDescription>
        void drawMeshIdx(
            const ptr<const MeshDescType_> meshDescription_,
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept {
            this->drawMeshIdx(
                static_cast<const ptr<const MeshDescription>>(meshDescription_),
                instanceCount_,
                instanceOffset_,
                primitiveCount_,
                primitiveOffset_
            );
        }

        virtual void drawDispatch(
            u32 instanceCount_,
            u32 instanceOffset_,
            u32 primitiveCount_,
            u32 primitiveOffset_
        ) noexcept = 0;
    };
}
