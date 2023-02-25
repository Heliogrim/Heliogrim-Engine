#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember::engine::input {
    enum class DragDropObjectType : u64 {
        eNone = 0x0,
        eFileType = ("Drop::File"_typeId).data,
        eTextType = ("Drop::Text"_typeId).data
    };

    struct DragDropObjectFilePayload;
    struct DragDropObjectTextPayload;

    union DragDropObjectPayload {
        ptr<DragDropObjectFilePayload> files;
        ptr<DragDropObjectTextPayload> text;
    };

    class DragDropObject {
    public:
        using this_type = DragDropObject;

    protected:
        DragDropObject();

    public:
        virtual ~DragDropObject() = default;

    protected:
        DragDropObjectType _type;
        DragDropObjectPayload _payload;

    public:
        virtual bool storeFiles(cref<Vector<string>> paths_) = 0;

        virtual bool storeText(cref<string> text_) = 0;
    };
}
