#pragma once
#include <Engine.GFX/Pipeline/LORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderStageNode.hpp>
#include <Engine.GFX/Renderer/__fwd.hpp>

namespace ember::engine::gfx::render {

    class RevDepthSharedNode :
        public RenderStageNode {
    public:
        using this_type = RevDepthSharedNode;

    public:
        RevDepthSharedNode();

        ~RevDepthSharedNode() override;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<HORenderPass> renderPass_) override;

        bool free(const ptr<HORenderPass> renderPass_) override;

    public:
        [[nodiscard]] Vector<RenderDataToken> requiredToken() noexcept override;

        [[nodiscard]] Vector<RenderDataToken> optionalToken() noexcept override;

    public:
        void before(const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_) const override;

        void invoke(const non_owning_rptr<HORenderPass> renderPass_, const non_owning_rptr<RenderStagePass> stagePass_,
            const non_owning_rptr<SceneNodeModel> model_) const override;

        void after(const non_owning_rptr<HORenderPass> renderPass_,
            const non_owning_rptr<RenderStagePass> stagePass_) const override;

    private:
        sptr<Device> _device;
        sptr<pipeline::LORenderPass> _loRenderPass;

    public:
        [[nodiscard]] cref<sptr<pipeline::LORenderPass>> loRenderPass() const noexcept;

    private:
        void setupLORenderPass();

        void destroyLORenderPass();

    private:
        void postProcessAllocated(const ptr<HORenderPass> renderPass_);
    };

}
