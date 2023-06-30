#include "Intermediate.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::acc::lang;
using namespace hg;

Intermediate::Intermediate(mref<uptr<IL>> il_, mref<uptr<IR>> ir_) noexcept :
    _code(_STD move(il_)),
    _rep(_STD move(ir_)) {}

Intermediate::Intermediate(cref<this_type> other_) :
    _code(clone(other_._code)),
    _rep(clone(other_._rep)) {}

nmpt<const IL> Intermediate::getIl() const noexcept {
    return _code.get();
}

void Intermediate::setIl(mref<uptr<IL>> il_) {
    _code = _STD move(il_);
}

nmpt<const IR> Intermediate::getIr() const noexcept {
    return _rep.get();
}

void Intermediate::setIr(mref<uptr<IR>> ir_) {
    _rep = _STD move(ir_);
}
