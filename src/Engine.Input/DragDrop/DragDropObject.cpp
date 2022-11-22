#include "DragDropObject.hpp"

using namespace ember::engine::input;
using namespace ember;

DragDropObject::DragDropObject() :
    _type(DragDropObjectType::eNone),
    _payload({ nullptr }) {}
