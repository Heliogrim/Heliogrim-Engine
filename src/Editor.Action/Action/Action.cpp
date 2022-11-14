#include "Action.hpp"

using namespace ember::editor;
using namespace ember;

Action::Action(cref<action_type_id> typeId_) :
    _typeId(typeId_) {}

const action_type_id Action::getTypeId() const noexcept {
    return _typeId;
}
