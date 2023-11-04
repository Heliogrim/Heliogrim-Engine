#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::render::cmd {
    class RenderCommandTranslationUnit;
}

namespace hg::engine::gfx::render::cmd {
    class __declspec(novtable) RenderCommand {
    public:
        using this_type = RenderCommand;

    public:
        constexpr RenderCommand() noexcept = default;

        constexpr ~RenderCommand() noexcept = default;

    public:
        virtual void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept = 0;
    };

    template <typename Type_>
    concept IsRenderCommand = _STD derived_from<Type_, RenderCommand> && _STD is_trivially_destructible_v<Type_>;
}
