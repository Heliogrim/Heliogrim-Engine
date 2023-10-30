#pragma once
#include <Engine.GFX.Render.Command/Buffer/RenderCommandBufferView.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../RuntimeComponent.hpp"
#include "../../Pass/IterationPassContext.hpp"

namespace hg::engine::gfx::render::graph {
    enum class RecordInvalidationResult {
        eNever,
        eAlways,
        /**/
        eNone,
        ePartialInvalidate,
        eInvalidated
    };

    class CommandRecordComponent :
        public InheritMeta<CommandRecordComponent, RuntimeComponent> {
    public:
        using this_type = CommandRecordComponent;

        using cmd_view = cmd::RenderCommandBufferView;
        using cmd_view_enum = Vector<cmd_view>;

    public:
        CommandRecordComponent() noexcept = default;

        ~CommandRecordComponent() noexcept override = default;

    public:
        [[nodiscard]] virtual RecordInvalidationResult iterate(cref<IterationPassContext> ctx_) = 0;

    public:
        [[nodiscard]] virtual cref<cmd_view_enum> getEnumeration() const noexcept = 0;

        template <typename Fn_>
        void enumerate(Fn_&& fn_) const noexcept {
            const auto& cvenum = getEnumeration();
            for (const auto& cv : cvenum) {
                fn_(clone(cv));
            }
        }

        template <typename Fn_>
        void enumerateFlatten(Fn_&& fn_) const noexcept {
            enumerate(
                [fn_ = _STD forward<Fn_>(fn_)](const auto& view_) {
                    for (const auto* const cmd : view_) {
                        fn_(cmd);
                    }
                }
            );
        }
    };
}
