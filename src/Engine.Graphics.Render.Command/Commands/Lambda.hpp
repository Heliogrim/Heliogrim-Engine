#pragma once
#include <Engine.Common/__macro.hpp>

#include "../RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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

        LambdaRCmd(mref<std::function<signature>> fn_) noexcept :
            fn(std::move(fn_)) {}

    private:
    public:
        std::function<signature> fn;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
