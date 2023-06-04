#pragma once
#include <Engine.GFX/FixedPipeline.hpp>
#include <Engine.GFX.Renderer/Node/RenderStageNode.hpp>
#include <Engine.GFX/Shader/ShaderBindingGroup.hpp>

namespace hg::engine::gfx {
    class VirtualTextureView;
}

namespace hg::engine::gfx::glow::render {
    class RevMainSharedNode;
}

namespace hg::engine::gfx::glow::render {
    class RevMainSkyNode :
        public gfx::render::RenderStageNode {
    public:
        using this_type = RevMainSkyNode;

    public:
        RevMainSkyNode(const ptr<RevMainSharedNode> sharedNode_);

        ~RevMainSkyNode() override = default;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<gfx::render::HORenderPass> renderPass_) override;

        bool free(const ptr<gfx::render::HORenderPass> renderPass_) override;

    public:
        [[nodiscard]] Vector<gfx::render::RenderDataToken> requiredToken() noexcept override;

        [[nodiscard]] Vector<gfx::render::RenderDataToken> optionalToken() noexcept override;

    public:
        [[nodiscard]] const non_owning_rptr<const Vector<type_id>> modelTypes() const noexcept override;

    public:
        void before(
            const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_
        ) const override;

        void invoke(
            const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_
        ) const override;

        void after(
            const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_
        ) const override;

    private:
        void setupShader(cref<sptr<Device>> device_);

    private:
        Vector<type_id> _modelTypes;

    private:
        sptr<Device> _device;

    private:
        ptr<RevMainSharedNode> _sharedNode;
        sptr<FixedPipeline> _pipeline;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;

    private:
        [[nodiscard]] const ptr<const VirtualTextureView> getDefaultSkybox() const;
    };
}
