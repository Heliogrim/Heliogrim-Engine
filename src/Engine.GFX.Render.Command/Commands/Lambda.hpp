#pragma once
#include "../RenderCommand.hpp"

#include <Engine.Common/Functional/Function.hpp>

namespace hg::engine::gfx::render::cmd {
    class ForwardRenderCommandBuffer;

    class LambdaRCmd :
        public RenderCommand {
    public:
        using this_type = LambdaRCmd;

        using signature = void (*)(ref<ForwardRenderCommandBuffer>);

    public:
        template <typename Fn_>
        constexpr LambdaRCmd(Fn_&& fn_) noexcept :
            fn(_STD forward<Fn_>(fn_)) {}

        constexpr LambdaRCmd(mref<_STD function<signature>> fn_) noexcept :
            fn(_STD move(fn_)) {}

    private:
    public:
        _STD function<signature> fn;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
