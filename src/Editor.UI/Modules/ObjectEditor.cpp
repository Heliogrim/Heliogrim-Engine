#include "ObjectEditor.hpp"

#include "../Panel/ObjectEditorPanel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ObjectEditor::ObjectEditor() :
    _panels(),
    _mapper() {}

ObjectEditor::~ObjectEditor() = default;

sptr<ObjectEditorPanel> ObjectEditor::makePanel() {
    auto panel = ObjectEditorPanel::make(this);
    _panels.push_back(panel);
    return panel;
}

bool ObjectEditor::storeObjectMapper(const type_id typeId_, mref<uptr<ObjectValueMapperBase>> mapper_) {

    if (_mapper.find(typeId_) != _mapper.end()) {
        return false;
    }

    _mapper.insert_or_assign(typeId_, _STD move(mapper_));
    return true;
}

ptr<ObjectValueMapperBase> ObjectEditor::getObjectMapper(const type_id typeId_) {

    const auto it = _mapper.find(typeId_);
    if (it != _mapper.end()) {
        return it->second.get();
    }

    return nullptr;
}
