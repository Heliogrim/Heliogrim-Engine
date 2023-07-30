#include "AccelerationEffect.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationEffect::AccelerationEffect(
    mref<Guid> guid_,
    mref<string> name_,
    mref<Vector<smr<Stage>>> stages_,
    mref<Vector<smr<Symbol>>> importSymbols_,
    mref<Vector<smr<Symbol>>> exportSymbols_
) :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _stages(_STD move(stages_)),
    _importSymbols(_STD move(importSymbols_)),
    _exportSymbols(_STD move(exportSymbols_)) {}

AccelerationEffect::~AccelerationEffect() = default;

Guid AccelerationEffect::getGuid() const noexcept {
    return _guid;
}

string AccelerationEffect::getName() const noexcept {
    return _name;
}

cref<Vector<smr<Stage>>> AccelerationEffect::getStages() const noexcept {
    return _stages;
}

cref<Vector<smr<Symbol>>> AccelerationEffect::getImportSymbols() const noexcept {
    return _importSymbols;
}

cref<Vector<smr<Symbol>>> AccelerationEffect::getExportSymbols() const noexcept {
    return _exportSymbols;
}
