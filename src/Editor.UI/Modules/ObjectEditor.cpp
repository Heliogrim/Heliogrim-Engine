#include "ObjectEditor.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

ObjectEditor::ObjectEditor() :
    _mapper() {}

ObjectEditor::~ObjectEditor() = default;

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
