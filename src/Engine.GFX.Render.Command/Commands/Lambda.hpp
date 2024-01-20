#pragma once
#include "../RenderCommand.hpp"

#include <Engine.Common/Functional/Function.hpp>

namespace hg::engine::accel {
    class AccelCommandBuffer;
}

namespace hg::engine::render::cmd {
    class LambdaRCmd :
        public RenderCommand {
    public:
        using this_type = LambdaRCmd;

        using signature = void(ref<accel::AccelCommandBuffer>);

    public:
        template <typename Fn_>
        LambdaRCmd(Fn_&& fn_) noexcept :
            fn(std::forward<Fn_>(fn_)) {}

        LambdaRCmd(mref<_STD function<signature>> fn_) noexcept :
            fn(std::move(fn_)) {}

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
