#include "MaterialPrototype.hpp"

#include <algorithm>
#include <ranges>

#include "Material.hpp"
#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

MaterialPrototype::MaterialPrototype(
	mref<Guid> guid_,
	mref<string> name_,
	mref<InlineAutoArray<MaterialEffect>> effects_,
	mref<Vector<MaterialPrototypeParameter>> parameters_
) noexcept :
	_guid(std::move(guid_)),
	_name(std::move(name_)),
	_materialEffects(std::move(effects_)),
	_parameters(std::move(parameters_)) {}

MaterialPrototype::~MaterialPrototype() = default;

Guid MaterialPrototype::getGuid() const noexcept {
	return _guid;
}

string MaterialPrototype::getName() const noexcept {
	return _name;
}

cref<InlineAutoArray<MaterialEffect>> MaterialPrototype::getMaterialEffects() const noexcept {
	return _materialEffects;
}

cref<Vector<MaterialPrototypeParameter>> MaterialPrototype::getParameters() const noexcept {
	return _parameters;
}

bool MaterialPrototype::addParameter(mref<MaterialPrototypeParameter> param_) {

	const auto iter = std::ranges::find(
		_parameters,
		param_.getId(),
		[](cref<MaterialPrototypeParameter> entry_) {
			return entry_.getId();
		}
	);

	if (iter != _parameters.end()) {
		return false;
	}

	_parameters.push_back(std::move(param_));
	return true;
}

tl::optional<cref<MaterialPrototypeParameter>> MaterialPrototype::getParameter(
	ParameterIdentifier identifier_
) const noexcept {
	const auto it = std::ranges::find(
		_parameters,
		identifier_,
		[](const auto& param_) {
			return param_.getId();
		}
	);

	return it != _parameters.end() ?
		tl::make_optional<cref<MaterialPrototypeParameter>>(*it) :
		tl::make_optional<cref<MaterialPrototypeParameter>>(tl::nullopt);
}

tl::optional<cref<MaterialPrototypeParameter>> MaterialPrototype::getFirstParameter(
	string_view name_
) const noexcept {

	const auto it = std::ranges::find_if(
		_parameters,
		[name_](const auto& param_) {
			return param_.getName() == name_;
		}
	);

	return it != _parameters.end() ?
		tl::make_optional<cref<MaterialPrototypeParameter>>(*it) :
		tl::make_optional<cref<MaterialPrototypeParameter>>(tl::nullopt);
}

bool MaterialPrototype::removeParameter(ParameterIdentifier identifier_) {

	const auto range = std::ranges::remove(
		_parameters,
		identifier_,
		[](const auto& param_) {
			return param_.getId();
		}
	);

	_parameters.erase(range.begin(), range.end());
	return range.begin() != range.end();
}

bool MaterialPrototype::removeFirstParameter(StringView name_) {

	const auto range = std::ranges::remove(
		_parameters,
		name_,
		[](const auto& param_) {
			return param_.getName();
		}
	);

	_parameters.erase(range.begin(), range.end());
	return range.begin() != _parameters.end();
}
