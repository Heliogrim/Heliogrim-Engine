#pragma once
#include "SLNode.hpp"

#include <Engine.GFX.Render.Subpass/SubPass.hpp>
#include "../../Symbol/ScopedSymbolContext.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SubPassNodeBase :
        public SLNode {
    public:
        using this_type = SubPassNodeBase;

    protected:
        SubPassNodeBase() noexcept = default;

    public:
        ~SubPassNodeBase() noexcept override = default;

    public:
        [[nodiscard]] virtual ptr<SubPass> subpass() noexcept = 0;
    };

    template <typename SubPassType_> requires _STD derived_from<SubPassType_, SubPass>
    class SubPassNode :
        public SubPassNodeBase {
    public:
        using this_type = SubPassNode<SubPassType_>;

        using subpass_type = SubPassType_;

    public:
        SubPassNode() noexcept = default;

        SubPassNode(mref<subpass_type> subpass_) noexcept :
            _subpass(_STD move(subpass_)) {
            // TODO: _subpass.setup();
        }

        ~SubPassNode() noexcept override {
            _subpass.destroy();
        }

    private:
        subpass_type _subpass;

    public:
        [[nodiscard]] ptr<subpass_type> subpass() noexcept override {
            return &_subpass;
        }

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override {
            _subpass.iterate();
            return IterationResultBits::eNone;
        }

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override {
            _subpass.resolve();
            return IterationResultBits::eNone;
        }

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override {
            _subpass.execute(static_cast<cref<ScopedSymbolContext>>(ctx_.symbols()));
            return IterationResultBits::eNone;
        }
    };
}
