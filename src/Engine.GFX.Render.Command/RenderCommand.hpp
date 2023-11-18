#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "RenderCommandTranslator.hpp"

namespace hg::engine::gfx::render::cmd {
    class __declspec(novtable) RenderCommand {
    public:
        using this_type = RenderCommand;

    public:
        constexpr RenderCommand() noexcept = default;

        virtual constexpr ~RenderCommand() noexcept = default;

    public:
        nmpt<RenderCommand> next;

    public:
        virtual void operator()(
            _In_ ptr<RenderCommandTranslator::State> state_,
            _In_ ptr<RenderCommandTranslator> translator_
        ) const noexcept = 0;
    };

    template <typename Type_>
    concept IsRenderCommand = _STD derived_from<Type_, RenderCommand> &&
        _STD is_nothrow_destructible_v<Type_> &&
        requires(const Type_& obj_) {
            obj_.operator()(
                _STD declval<ptr<RenderCommandTranslator::State>>(),
                _STD declval<ptr<RenderCommandTranslator>>()
            );
        };
}
