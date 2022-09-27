#pragma once
#include <Engine.GFX/Pipeline/LORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderStageNode.hpp>
#include <Engine.GFX/Renderer/__fwd.hpp>

namespace ember::engine::gfx::glow::render {

    class RevMainSharedNode :
        public gfx::render::RenderStageNode {
    public:
        using this_type = RevMainSharedNode;

    public:
        RevMainSharedNode();

        ~RevMainSharedNode() override;

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
        void before(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const override;

        void invoke(const non_owning_rptr<gfx::render::HORenderPass> renderPass_, const non_owning_rptr<gfx::render::RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_) const override;

        void after(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
            const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const override;

    private:
        sptr<Device> _device;
        sptr<pipeline::LORenderPass> _loRenderPass;

    public:
        [[nodiscard]] cref<sptr<pipeline::LORenderPass>> loRenderPass() const noexcept;

    private:
        void setupLORenderPass();

        void destroyLORenderPass();

    private:
        void postProcessAllocated(const ptr<gfx::render::HORenderPass> renderPass_) const;
    };

}
