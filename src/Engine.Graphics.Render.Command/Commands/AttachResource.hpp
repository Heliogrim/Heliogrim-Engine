#pragma once

#include "../RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import Engine.Reflect;
#endif

namespace hg::engine::render::cmd {
    class AttachResourceRCmd :
        public RenderCommand {
    public:
        AttachResourceRCmd() noexcept;

        ~AttachResourceRCmd() noexcept override = default;

    private:
    public:
        smr<ClassMetaBase> _attached;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;

    public:
        [[nodiscard]] smr<ClassMetaBase> attached() const noexcept;
    };
}
