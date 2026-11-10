#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"
#include "../Resource/ResourceTable.hpp"

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
