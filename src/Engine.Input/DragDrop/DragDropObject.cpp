#include "DragDropObject.hpp"

using namespace hg::engine::input;
using namespace hg;

DragDropObject::DragDropObject() :
    _type(DragDropObjectType::eNone),
    _payload({ nullptr }) {}
