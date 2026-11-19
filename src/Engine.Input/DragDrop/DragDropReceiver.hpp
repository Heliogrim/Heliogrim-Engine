#pragma once

#include <Engine.Common/__macro.hpp>

#include "../DragDropEvent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::input {
    class macro_novtable DragDropReceiver {
    public:
        using this_type = DragDropReceiver;

    protected:
        DragDropReceiver() = default;

    public:
        virtual ~DragDropReceiver() noexcept = default;

    public:
        virtual void setup() = 0;

        virtual void destroy() = 0;

    protected:
        std::function<bool(mref<uptr<event::DragDropEvent>> event_)> _onDrag;
        std::function<bool(mref<uptr<event::DragDropEvent>> event_)> _onDrop;
        std::function<bool(mref<uptr<event::DragDropEvent>> event_)> _onMove;

    public:
        void setOnDrag(mref<decltype(_onDrag)> onDrag_);

        void setOnDrop(mref<decltype(_onDrag)> onDrop_);

        void setOnMove(mref<decltype(_onDrag)> onMove_);
    };
}
