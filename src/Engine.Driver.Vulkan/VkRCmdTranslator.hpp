#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandTranslator.hpp>

#include "VkCmdMgr.hpp"
#include "VkNativeBatch.hpp"
#include "VkScopedResourceTable.hpp"
#include "VkScopedCmdMgr.hpp"

namespace hg::driver::vk {
    class VkRCmdTranslator final :
        public engine::render::cmd::RenderCommandTranslator {
    public:
        using this_type = VkRCmdTranslator;
        using base_type = engine::render::cmd::RenderCommandTranslator;

    public:
        [[nodiscard]] uptr<engine::render::cmd::NativeBatch> operator()(
            const ptr<const engine::render::cmd::RenderCommandBuffer> commands_
        ) noexcept override;

    public:
        class VkState :
            public State {
        public:
            using this_type = VkState;

        public:
            VkState(mref<VkScopedCmdMgr> cmd_) noexcept :
                State(),
                cmd(_STD move(cmd_)) {}

            ~VkState() noexcept = default;

        public:
            VkScopedCmdMgr cmd;
            VkScopedResourceTable srt;
        };

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::RenderCommand>
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BeginRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BeginAccelerationPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BeginSubPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::NextSubpassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::EndSubPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::EndAccelerationPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::EndRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindPipelineRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindResourceTableRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindIndexBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindVertexBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindSkeletalMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindStaticMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindStorageBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindTextureRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindTextureSamplerRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::BindUniformBufferRCmd>
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::DrawMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::DrawSkeletalMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::DrawDispatchRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::DrawDispatchIndirectRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::LambdaRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::render::cmd::AttachResourceRCmd> cmd_
        ) noexcept override;
    };
}
