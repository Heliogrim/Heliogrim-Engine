#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Meta/Concept.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "ObjectEditor/ObjectValueMapper.hpp"

namespace ember::editor::ui {
    class ObjectEditorPanel;
}

namespace ember::editor::ui {
    class ObjectEditor {
    public:
        using this_type = ObjectEditor;

    public:
        ObjectEditor();

        ~ObjectEditor();

    private:
    public:
        Vector<wptr<ObjectEditorPanel>> _panels;

    public:
        [[nodiscard]] sptr<ObjectEditorPanel> makePanel();

    private:
        ska::bytell_hash_map<type_id, uptr<ObjectValueMapperBase>> _mapper;

    public:
        bool storeObjectMapper(const type_id typeId_, mref<uptr<ObjectValueMapperBase>> mapper_);

        template <HasStaticType ObjectType_>
        bool storeTypeMapper(mref<uptr<ObjectValueMapper<ObjectType_>>> mapper_) {
            return storeObjectMapper(ObjectType_::type_id, _STD move(mapper_));
        }

        [[nodiscard]] ptr<ObjectValueMapperBase> getObjectMapper(const type_id typeId_);

        template <HasStaticType ObjectType_>
        [[nodiscard]] ptr<ObjectValueMapper<ObjectType_>> getTypeMapper() {
            return getObjectMapper(ObjectType_::type_id);
        }
    };
}
