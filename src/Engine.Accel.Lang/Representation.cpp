#include "Representation.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::accel;
using namespace hg;

lang::Representation::Representation(cref<this_type> other_) :
    globalScope(),
    mainEntry(),
    symbolTable() {
    _STD unreachable();
    // TODO: Deep copy global scope
    // TODO: Query mainEntry in new scope
    // TODO: Rebuild symbol table
}
