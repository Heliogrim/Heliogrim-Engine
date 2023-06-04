#pragma once
#include <Engine.GFX/FixedPipeline.hpp>
#include <Engine.GFX/Pipeline/LORenderPass.hpp>
#include <Engine.GFX.Renderer/Node/RenderStageNode.hpp>
#include <Engine.GFX/Shader/ShaderBindingGroup.hpp>

namespace hg::editor::gfx {
    class EdGridNode :
        public engine::gfx::render::RenderStageNode {
    public:
        using this_type = EdGridNode;

    public:
        EdGridNode();

        ~EdGridNode() override;

    public:
        void setup(cref<sptr<engine::gfx::Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<engine::gfx::render::HORenderPass> renderPass_) override;

        bool free(const ptr<engine::gfx::render::HORenderPass> renderPass_) override;

    public:
        [[nodiscard]] Vector<engine::gfx::render::RenderDataToken> requiredToken() noexcept override;

        [[nodiscard]] Vector<engine::gfx::render::RenderDataToken> optionalToken() noexcept override;

    public:
        [[nodiscard]] const non_owning_rptr<const Vector<type_id>> modelTypes() const noexcept override;

    public:
        void before(
            const non_owning_rptr<engine::gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<engine::gfx::render::RenderStagePass> stagePass_
        ) const override;

        void invoke(
            const non_owning_rptr<engine::gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<engine::gfx::render::RenderStagePass> stagePass_,
            const non_owning_rptr<engine::gfx::SceneNodeModel> model_
        ) const override;

        void after(
            const non_owning_rptr<engine::gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<engine::gfx::render::RenderStagePass> stagePass_
        ) const override;

    private:
        sptr<engine::gfx::Device> _device;
        sptr<engine::gfx::pipeline::LORenderPass> _loRenderPass;

    private:
        void setupLORenderPass();

        void destroyLORenderPass();

    private:
        Vector<vk::DescriptorPoolCreateInfo> _reqDescPools;
        Vector<engine::gfx::shader::ShaderBindingGroup> _reqBindGroups;

    private:
        void setupShader();

    private:
        sptr<engine::gfx::FixedPipeline> _pipeline;

    private:
        void setupPipeline();
    };
}
