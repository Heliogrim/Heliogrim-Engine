#include "DataLayoutBase.hpp"

#include <cassert>

#include "LayoutDefine.hpp"

using namespace hg::engine::serialization;
using namespace hg;

DataLayoutBase::DataLayoutBase(const bool hasReflectedClass_) :
	DataLayoutReflect(),
	DataLayoutDispatch(*this),
	_hasReflectedClass(hasReflectedClass_) {}

DataLayoutBase::~DataLayoutBase() {

	tidy();

	DataLayoutReflect::~DataLayoutReflect();
	DataLayoutDispatch::~DataLayoutDispatch();
}

void DataLayoutBase::tidy() {
	_defines.clear();
}

cref<DataLayoutReflect> DataLayoutBase::reflect() const noexcept {
	return static_cast<cref<DataLayoutReflect>>(*this);
}

ref<DataLayoutReflect> DataLayoutBase::reflect() noexcept {
	return static_cast<ref<DataLayoutReflect>>(*this);
}

cref<DataLayoutDispatch> DataLayoutBase::dispatch() const noexcept {
	return static_cast<cref<DataLayoutDispatch>>(*this);
}

bool DataLayoutBase::hasReflectedClass() const noexcept {
	return _hasReflectedClass;
}

bool DataLayoutBase::hasDynamicDefines() const noexcept {

	for (const auto& entry : _defines) {
		if (entry.hasSubDefines()) {
			return true;
		}
	}

	return false;
}

bool DataLayoutBase::justConstantDefines() const noexcept {

	for (const auto& entry : _defines) {
		if (entry.hasSubDefines()) {
			return false;
		}
	}

	return true;
}

s64 DataLayoutBase::size() const noexcept {

	s64 size { 0LL };
	for (const auto& entry : _defines) {

		const auto last { static_cast<s64>(entry._offset) + entry.size() };
		assert(last != entry._offset);

		size = MAX(size, last);
	}

	return size;
}

cref<decltype(DataLayoutBase::_defines)> DataLayoutBase::defines() const noexcept {
	return _defines;
}

void DataLayoutBase::define(mref<layout::LayoutDefine> define_) {
	_defines.emplace_back(std::move(define_));
}
