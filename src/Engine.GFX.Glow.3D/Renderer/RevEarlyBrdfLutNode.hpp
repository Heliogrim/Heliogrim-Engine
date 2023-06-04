#pragma once
#include <Engine.GFX/FixedPipeline.hpp>
#include <Engine.GFX.Renderer/Node/RenderStageNode.hpp>
#include <Engine.GFX/Shader/ShaderBindingGroup.hpp>
#include <Engine.GFX/Pipeline/LORenderPass.hpp>
#include <Engine.GFX/Texture/__fwd.hpp>

namespace hg::engine::gfx::glow::render {
    class RevEarlyBrdfLutNode :
        public gfx::render::RenderStageNode {
    public:
        using this_type = RevEarlyBrdfLutNode;

    public:
        RevEarlyBrdfLutNode();

        ~RevEarlyBrdfLutNode() override = default;

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
        Vector<type_id> _modelTypes;

    private:
        sptr<Device> _device;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;

    private:
        math::uivec2 _brdfLutExtent;
        sptr<Texture> _brdfLut;

    private:
        sptr<pipeline::LORenderPass> setupLORenderPass();

        void destroyLORenderPass(mref<ptr<pipeline::LORenderPass>> renderPass_);

        [[nodiscard]] Framebuffer setupFramebuffer(cref<sptr<pipeline::LORenderPass>> renderPass_);

        void destroyFramebuffer(mref<Framebuffer> framebuffer_);
    };
}
