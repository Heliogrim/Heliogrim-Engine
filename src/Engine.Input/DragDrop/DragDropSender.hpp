#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::input {
    class macro_novtable DragDropSender {
    public:
        using this_type = DragDropSender;

    protected:
        DragDropSender() = default;

    public:
        virtual ~DragDropSender() noexcept = default;

    public:
        virtual void setup() = 0;

        virtual void destroy() = 0;

    public:
        virtual void sendDragFiles(Vector<string> paths_) = 0;

        virtual void sendDragText(cref<string> text_) = 0;
    };
}
