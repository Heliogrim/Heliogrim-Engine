#pragma once

#include <Engine.Common/__macro.hpp>

#include "../RenderCommand.hpp"
#include "../Resource/ResourceTable.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::cmd {
    class BindResourceTableRCmd :
        public RenderCommand {
    public:
        using this_type = BindResourceTableRCmd;

    public:
        BindResourceTableRCmd(mref<smr<const ResourceTable>> table_) noexcept :
            RenderCommand(),
            _table(std::move(table_)) {}

        ~BindResourceTableRCmd() noexcept override = default;

    private:
    public:
        smr<const ResourceTable> _table;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
