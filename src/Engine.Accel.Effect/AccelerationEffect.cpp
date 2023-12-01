#include "AccelerationEffect.hpp"

#include <algorithm>

using namespace hg::engine::accel;
using namespace hg;

AccelerationEffect::AccelerationEffect(
    mref<Guid> guid_,
    mref<string> name_,
    mref<Vector<smr<Stage>>> stages_
) :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _stages(_STD move(stages_)) {}

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

void AccelerationEffect::enumerateImportSymbols(ref<Vector<StageInput>> imports_) const noexcept {

    Vector<StageInput> tmpIn {};
    Vector<StageOutput> tmpOut {};

    for (const auto& stage : _stages) {

        stage->enumerateStageInputs(tmpIn);

        auto tmpBeg = tmpIn.begin();
        auto tmpEnd = tmpIn.end();

        for (const auto& out : tmpOut) {

            const auto result = _STD ranges::remove_if(
                tmpBeg,
                tmpEnd,
                [&out](const auto& stageIn) {
                    if (out.symbol == stageIn.symbol || out.symbol->name == stageIn.symbol->name) {
                        return true;
                    }
                    return false;
                }
            );

            tmpBeg = result.begin();
            tmpEnd = result.end();
        }

        /**/

        imports_.insert(imports_.end(), tmpBeg, tmpEnd);
        tmpIn.clear();
        tmpOut.clear();

        /**/

        stage->enumerateStageOutputs(tmpOut);
    }
}

void AccelerationEffect::enumerateExportSymbols(ref<Vector<StageOutput>> exports_) const noexcept {

    Vector<StageInput> tmpIn {};
    Vector<StageOutput> tmpOut {};

    for (const auto& stage : _stages) {

        stage->enumerateStageInputs(tmpIn);

        auto tmpBeg = tmpOut.begin();
        auto tmpEnd = tmpOut.end();

        for (const auto& in : tmpIn) {

            const auto result = _STD ranges::remove_if(
                tmpBeg,
                tmpEnd,
                [&in](const auto& stageOut) {
                    if (in.symbol == stageOut.symbol || in.symbol->name == stageOut.symbol->name) {
                        return true;
                    }
                    return false;
                }
            );

            tmpBeg = result.begin();
            tmpEnd = result.end();
        }

        /**/

        exports_.insert(exports_.end(), tmpBeg, tmpEnd);
        tmpIn.clear();
        tmpOut.clear();

        /**/

        stage->enumerateStageOutputs(tmpOut);
    }

    if (not tmpOut.empty()) {
        exports_.insert(exports_.end(), tmpOut.begin(), tmpOut.end());
    }
}

Optional<StageInput> AccelerationEffect::getFirstInputFor(cref<lang::Symbol> symbol_) const noexcept {

    Vector<StageInput> tmp {};
    for (const auto& stage : _stages) {

        tmp.clear();
        stage->enumerateStageInputs(tmp);

        for (const auto& stageIn : tmp) {
            if (stageIn.symbol->name == symbol_.name) {
                return tl::make_optional(stageIn);
            }
        }
    }

    return { tl::nullopt };
}

Optional<StageOutput> AccelerationEffect::getLastOutputFor(cref<lang::Symbol> symbol_) const noexcept {

    Vector<StageOutput> tmp {};
    for (const auto& stage : _stages) {

        tmp.clear();
        stage->enumerateStageOutputs(tmp);

        for (const auto& stageOut : tmp) {
            if (stageOut.symbol->name == symbol_.name) {
                return tl::make_optional(stageOut);
            }
        }
    }

    return { tl::nullopt };
}
