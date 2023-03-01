#pragma once
#include <Engine.GFX/FixedPipeline.hpp>
#include <Engine.GFX/Renderer/RenderStageNode.hpp>
#include <Engine.GFX/Shader/ShaderBindingGroup.hpp>
#include <Engine.GFX/Pipeline/LORenderPass.hpp>
#include <Engine.GFX/Texture/__fwd.hpp>

namespace hg::engine::gfx::glow::render {
    class RevEarlyEnvPrefilterNode :
        public gfx::render::RenderStageNode {
    public:
        using this_type = RevEarlyEnvPrefilterNode;

    public:
        RevEarlyEnvPrefilterNode();

        ~RevEarlyEnvPrefilterNode() override = default;

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
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;

    private:
        math::uivec2 _envPrefilterExtent;
        TextureFormat _envPrefilterFormat;

    private:
        sptr<pipeline::LORenderPass> _loRenderPass;

    private:
        void setupLORenderPass();

        void destroyLORenderPass();

    private:
        sptr<FixedPipeline> _pipeline;

    private:
        void setupPipeline();

    private:
        [[nodiscard]] Framebuffer allocateFramebuffer();

        void freeFramebuffer(mref<Framebuffer> framebuffer_);

    private:
        void postProcessAllocated(const ptr<gfx::render::HORenderPass> renderPass_) const;
    };
}
