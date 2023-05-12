#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>

#include "Widget/__fwd.hpp"
#include "Widget/WidgetState.hpp"
#include "ReflowPassState.hpp"

namespace hg::engine::reflow {
    class ReflowState {
    public:
        using this_type = ReflowState;

    public:
        ReflowState();

        ~ReflowState();

    public:
        void reset();

    private:
        DenseMap<ptr<void>, non_owning_rptr<ReflowPassState>> _recorded;

    public:
        [[nodiscard]] non_owning_rptr<ReflowPassState> record(cref<sptr<Widget>> widget_);

        [[nodiscard]] const non_owning_rptr<const ReflowPassState> getStateOf(cref<sptr<Widget>> widget_) const;

        [[nodiscard]] const non_owning_rptr<const ReflowPassState> getStateOf(
            const non_owning_rptr<const Widget> widget_
        ) const;

        [[nodiscard]] const non_owning_rptr<ReflowPassState> getStateOf(cref<sptr<Widget>> widget_);
    };
}
