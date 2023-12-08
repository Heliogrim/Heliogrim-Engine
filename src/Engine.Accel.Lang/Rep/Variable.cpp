#include "Variable.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

Variable::Variable(cref<this_type> other_) :
    type(other_.type),
    annotation(nullptr),
    countTag(other_.countTag) {
    if (other_.annotation != nullptr) {
        annotation = uptr<Annotation>(other_.annotation->createCopy());
    }
}
