#pragma once
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Acc/Command/CommandBuffer.hpp>

#include "ExecutionTranslationUnit.hpp"

namespace hg::engine::gfx::render::graph {
    class VkExecutionTranslationUnit :
        public ExecutionTranslationUnit {
    public:
        using this_type = VkExecutionTranslationUnit;

    public:
        VkExecutionTranslationUnit(mref<uptr<Resolver>> resolver_) noexcept;

        ~VkExecutionTranslationUnit() noexcept override;

    private:
        Vector<Vector<AccelerationCommandBuffer>> _committed;
        Vector<AccelerationCommandBuffer> _building;

    private:
        enum class InversionControlType {
            eDirect,
            eMaterial
        };

        struct InversionControl {
            InversionControlType type;
            ptr<const void> data;
        };

        Vector<InversionControl> _inversions;

    private:
        struct BoundDispatchContext {
            InlineAutoArray<ptr<const IndexBufferView>, 1> indexBuffer;
            InlineAutoArray<ptr<const VertexBufferView>, 2> vertexBuffer;
        };

        BoundDispatchContext _bdctx;

    public:
        void translate(ptr<const cmd::BeginRenderCommand>) noexcept override;

        void translate(ptr<const cmd::NextSubpassRenderCommand>) noexcept override;

        void translate(ptr<const cmd::EndRenderCommand>) noexcept override;

    public:
        void translate(ptr<const cmd::BindIndexBufferRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindVertexBufferRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindMaterialRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindSkeletalMeshRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindStaticMeshRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindStorageBufferRenderCommand>) noexcept override;

        void translate(ptr<const cmd::BindTextureRenderCommand>) noexcept override;

    public:
        void translate(ptr<const cmd::DrawMeshRenderCommand>) noexcept override;

        void translate(ptr<const cmd::DrawSkeletalMeshRenderCommand>) noexcept override;
    };
}
