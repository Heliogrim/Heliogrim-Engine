#pragma once
#include <Engine.GFX/ComputePipeline.hpp>
#include <Engine.GFX/Renderer/RenderStageNode.hpp>
#include <Engine.GFX/Shader/ShaderBindingGroup.hpp>

#include "State/__fwd.hpp"

namespace ember::engine::gfx::glow::render {
    class RevEarlySFNode final :
        public gfx::render::RenderStageNode {
    public:
        using this_type = RevEarlySFNode;

    public:
        RevEarlySFNode();

        ~RevEarlySFNode() override = default;

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
        sptr<Device> _device;

    private:
        sptr<ComputePipeline> _pipeline;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;

    private:
        void postProcessAllocated(const ptr<gfx::render::HORenderPass> renderPass_);

        [[nodiscard]] bool rebuildMttBuffer(
            cref<sptr<RevSfMtt>> sfMtt_,
            cref<sptr<gfx::render::RenderPassState>> state_
        ) const;

        [[nodiscard]] bool rebuildCsfmBuffer(
            cref<sptr<RevSfMtt>> sfMtt_,
            cref<sptr<gfx::render::RenderPassState>> state_
        ) const;
    };
}
