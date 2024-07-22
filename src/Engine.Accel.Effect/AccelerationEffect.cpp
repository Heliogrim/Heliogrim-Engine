#include "AccelerationEffect.hpp"

#include <algorithm>

using namespace hg::engine::accel;
using namespace hg;

AccelerationEffect::AccelerationEffect(
	mref<Guid> guid_,
	mref<string> name_,
	mref<Vector<smr<Stage>>> stages_
) :
	_guid(std::move(guid_)),
	_name(std::move(name_)),
	_stages(std::move(stages_)) {}

AccelerationEffect::~AccelerationEffect() = default;

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

			const auto result = std::ranges::remove_if(
				tmpBeg,
				tmpEnd,
				[&out](const auto& stageIn) {
					if (out.symbol == stageIn.symbol || out.symbol->symbolId == stageIn.symbol->symbolId) {
						return true;
					}
					return false;
				}
			);

			tmpBeg = tmpIn.begin();
			tmpEnd = result.begin();
		}

		tmpIn.erase(tmpEnd, tmpIn.end());

		/**/

		imports_.insert_range(imports_.end(), tmpIn);
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

			const auto result = std::ranges::remove_if(
				tmpBeg,
				tmpEnd,
				[&in](const auto& stageOut) {
					if (in.symbol == stageOut.symbol || in.symbol->symbolId == stageOut.symbol->symbolId) {
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
			if (stageIn.symbol->symbolId == symbol_.symbolId) {
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
			if (stageOut.symbol->symbolId == symbol_.symbolId) {
				return tl::make_optional(stageOut);
			}
		}
	}

	return { tl::nullopt };
}
