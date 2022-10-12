#pragma once
#include <Engine.GFX/Renderer/RenderStageNode.hpp>
#include <Engine.GFX/Renderer/__fwd.hpp>

#include "Engine.GFX/FixedPipeline.hpp"
#include "Engine.GFX/Shader/ShaderBindingGroup.hpp"
#include <Engine.GFX/Pipeline/LORenderPass.hpp>

#include "Engine.GFX/Texture/Texture.hpp"

#if TRUE
#include <mutex>
extern _STD mutex uiTestMtx;
#endif

namespace ember::engine::gfx::glow::ui::render {

    class UiMainStageNode final :
        public gfx::render::RenderStageNode {
    public:
        using this_type = UiMainStageNode;

    public:
        UiMainStageNode();

        ~UiMainStageNode() override;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<gfx::render::HORenderPass> renderPass_) override;

        bool free(const ptr<gfx::render::HORenderPass> renderPass_) override;

    public:
        Vector<gfx::render::RenderDataToken> requiredToken() noexcept override;

        Vector<gfx::render::RenderDataToken> optionalToken() noexcept override;

    public:
        void before(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const override;

        void invoke(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_) const override;

        void after(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const override;

    private:
        sptr<Device> _device;

    private:
        sptr<pipeline::LORenderPass> _loRenderPass;
        sptr<FixedPipeline> _pipeline;

    private:
        Vector<vk::DescriptorPoolCreateInfo> _requiredDescriptorPools;
        Vector<shader::ShaderBindingGroup> _requiredBindingGroups;

    private:
        void setupShader();

        void setupPipeline();

        void setupLORenderPass();

        void destroyLORenderPass();

    private:
        [[nodiscard]] Framebuffer allocateFramebuffer(const non_owning_rptr<gfx::render::HORenderPass> renderPass_);

        void freeFramebuffer(mref<Framebuffer> framebuffer_);

    private:
        Texture _defaultImage;

        void setupDefaultImage();

        void destroyDefaultImage();

        void rebuildImageDescriptors(ref<sptr<vk::DescriptorPool>> pool_,
            ref<sptr<Vector<shader::DiscreteBindingGroup>>> groups_, u32 count_) const;
    };

}