#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandTranslator.hpp>

#include "VkCmdMgr.hpp"
#include "VkNativeBatch.hpp"
#include "VkScopedCmdMgr.hpp"

namespace hg::driver::vk {
    class VkRCmdTranslator final :
        public engine::gfx::render::cmd::RenderCommandTranslator {
    public:
        using this_type = VkRCmdTranslator;
        using base_type = engine::gfx::render::cmd::RenderCommandTranslator;

    public:
        [[nodiscard]] ptr<VkNativeBatch> operator()(
            const ptr<const engine::gfx::render::cmd::RenderCommandBuffer> commands_
        ) noexcept override;

    public:
        class VkState :
            public State {
        public:
            using this_type = VkState;

        public:
            constexpr VkState(mref<VkScopedCmdMgr> cmd_) noexcept :
                State(),
                cmd(_STD move(cmd_)) {}

            constexpr ~VkState() noexcept = default;

        public:
            VkScopedCmdMgr cmd;
        };

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::RenderCommand>
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BeginRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BeginAccelerationPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BeginSubPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::NextSubpassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::EndSubPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::EndAccelerationPassRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::EndRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindPipelineRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindResourceTableRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindIndexBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindVertexBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindSkeletalMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindStaticMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindStorageBufferRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::BindTextureRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::DrawMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::DrawSkeletalMeshRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::DrawDispatchRCmd> cmd_
        ) noexcept override;

        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::DrawDispatchIndirectRCmd> cmd_
        ) noexcept override;

    public:
        void translate(
            ptr<State> state_,
            ptr<const engine::gfx::render::cmd::LambdaRCmd> cmd_
        ) noexcept override;
    };
}
