#pragma once

#include <Engine.Reflow/Widget/Panel.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>

#include "../Modules/ObjectEditor.hpp"

namespace hg::editor::ui {
    class ObjectEditorPanel :
        public engine::reflow::VerticalPanel {
    public:
        using this_type = ObjectEditorPanel;

    protected:
        ObjectEditorPanel();

    public:
        ~ObjectEditorPanel() override;

    private:
        sptr<Widget> _nav;
        sptr<engine::reflow::VScrollBox> _content;

    private:
    public:
        non_owning_rptr<ObjectEditor> _editor;
        non_owning_rptr<ObjectValueMapperBase> _mapper;

        non_owning_rptr<void> _mappedTarget;

    public:
        void setEditorTarget(cref<type_id> typeId_, const ptr<void> obj_);

        void clearEditor();

    public:
        [[nodiscard]] static sptr<ObjectEditorPanel> make(const non_owning_rptr<ObjectEditor> editor_);
    };
}
