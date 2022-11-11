#pragma once

#include <Engine.Reflow/Widget/Panel.hpp>

#include "../Modules/ObjectEditor.hpp"

namespace ember::editor::ui {

    class ObjectEditorPanel :
        public engine::reflow::Panel {
    public:
        using this_type = ObjectEditorPanel;

    protected:
        ObjectEditorPanel();

    public:
        ~ObjectEditorPanel() override;

    private:
    public:
        non_owning_rptr<ObjectEditor> _editor;
        non_owning_rptr<ObjectValueMapperBase> _mapper;

    public:
        void setEditorTarget(cref<type_id> typeId_, const ptr<void> obj_);

        void clearEditor();

    public:
        [[nodiscard]] static sptr<ObjectEditorPanel> make(const non_owning_rptr<ObjectEditor> editor_);
    };

}
