#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace ember::engine::input {

    class __declspec(novtable) DragDropSender {
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
