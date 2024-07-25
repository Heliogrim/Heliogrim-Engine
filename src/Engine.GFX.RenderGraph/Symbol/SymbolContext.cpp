#include "SymbolContext.hpp"

#include <ranges>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render::graph;
using namespace hg;

SymbolContext::SymbolContext() noexcept = default;

SymbolContext::~SymbolContext() noexcept {
	tidy();
}

void SymbolContext::tidy() {

	for (const auto value : _resources | std::views::values) {

		if (value->owned) {
			// Warning: Externally owned resource have to be release manually e.g. by it's provider
			// Error: We may get into problems here, cause we can't guarantee the resource to be isolated
			// assert(value->data == nullptr); // We are not allowed to do this, cause owned resource release may be deferred
		}

		if (not value->owned) {
			// TODO: Handle destruction of contextualized resources
			assert(value->data == nullptr);
			const auto* mem = value.get();
			delete mem;
		}

	}

	_resources.clear();
}

bool SymbolContext::isRoot() const noexcept {
	return _parent == nullptr;
}

nmpt<const SymbolContext::this_type> SymbolContext::root() const noexcept {
	if (_parent != nullptr) {
		return _parent->root();
	}
	return this;
}

nmpt<SymbolContext::this_type> SymbolContext::parent() const noexcept {
	return _parent;
}

void SymbolContext::exposeSymbol(mref<smr<const Symbol>> symbol_, nmpt<SymbolizedResource> resource_) {

	// Warning: Temporary
	assert(not _resources.contains(symbol_));

	_resources.emplace(std::move(symbol_), std::move(resource_));
}

nmpt<SymbolizedResource> SymbolContext::exportSymbol(mref<smr<const Symbol>> symbol_) {

	// Warning: Temporary
	assert(not _resources.contains(symbol_));

	mpt<SymbolizedResource> resource = new SymbolizedResource {
		false, false, clone(symbol_), nullptr
	};

	_resources.emplace(std::move(symbol_), nmpt<SymbolizedResource> { resource });
	return resource;
}

nmpt<SymbolizedResource> SymbolContext::importSymbol(mref<smr<const Symbol>> symbol_) {

	// Warning: Temporary
	assert(_resources.contains(symbol_));

	const auto iter = _resources.find(symbol_);
	return iter != _resources.end() ? iter->second : nullptr;
}

bool SymbolContext::registerExposeSymbol(
	mref<smr<const Symbol>> symbol_,
	const ptr<Observed<SymbolizedResource>> observation_
) {

	auto iter = std::ranges::find_if(
		_register,
		[&symbol_](const auto& pair_) {
			return pair_.first == symbol_;
		}
	);

	if (iter == _register.end()) {
		_register.emplace_back(std::make_pair(clone(symbol_), decltype(_register)::value_type::second_type {}));
		iter = --_register.end();
	}

	iter->second.push_back({ observation_ });
	return true;
}

bool SymbolContext::registerExportSymbol(
	mref<smr<const Symbol>> symbol_,
	const ptr<Subscribed<SymbolizedResource>> subscription_
) {
	return false;
}

bool SymbolContext::registerImportSymbol(
	mref<smr<const Symbol>> symbol_,
	const ptr<Subscribed<SymbolizedResource>> subscription_
) {
	return false;
}

nmpt<SymbolizedResource> SymbolContext::getExportSymbol(mref<smr<const Symbol>> symbol_) const {
	return getImportSymbol(std::move(symbol_));
}

nmpt<SymbolizedResource> SymbolContext::getImportSymbol(mref<smr<const Symbol>> symbol_) const {

	if (const auto iter = _resources.find(symbol_); iter != _resources.end()) {
		return iter->second;
	}

	/**/

	if (_parent != nullptr) {
		return _parent->getImportSymbol(std::move(symbol_));
	}

	return nullptr;
}

bool SymbolContext::eraseExposedSymbol(mref<smr<const Symbol>> symbol_) {

	if (not _resources.contains(symbol_)) {
		return false;
	}

	const auto iter = _resources.find(symbol_);
	auto stored = iter->second;

	if (not stored->owned) {
		IM_CORE_ERROR("Tried to erase resource as exposed symbol, while subject is internally owned.");
		::hg::breakpoint();
		return false;
	}

	// As we are dealing with exposed symbols, we don't need to guarantee an empty data state

	_resources.erase(iter);
	return true;
}

bool SymbolContext::eraseExposedSymbol(mref<nmpt<SymbolizedResource>> resource_) {

	const auto& symbol = resource_->symbol;
	if (_resources.contains(symbol)) {
		return false;
	}

	const auto iter = _resources.find(symbol);
	const auto stored = iter->second;

	if (stored != resource_) {
		IM_CORE_ERROR("Unexpected stored subject while trying to erase exposed symbolized resource.");
		::hg::breakpoint();
		return false;
	}

	// As we are dealing with exposed symbols, we don't need to guarantee an empty data state

	_resources.erase(iter);
	return true;
}

bool SymbolContext::eraseExportSymbol(mref<smr<const Symbol>> symbol_) {

	if (not _resources.contains(symbol_)) {
		return false;
	}

	const auto iter = _resources.find(symbol_);
	auto stored = iter->second;

	if (stored->owned) {
		IM_CORE_ERROR("Tried to erase resource as export symbol, while subject is externaly owned.");
		::hg::breakpoint();
		return false;
	}

	if (stored->data != nullptr) {
		IM_CORE_ERROR("Encountered invalid data state (!= nullptr) while erasing subject.");
		::hg::breakpoint();
		return false;
	}

	_resources.erase(iter);

	const auto* mem = stored.get();
	delete mem;

	return true;
}

bool SymbolContext::eraseExportSymbol(mref<nmpt<SymbolizedResource>> resource_) {

	const auto& symbol = resource_->symbol;
	if (not _resources.contains(symbol)) {
		return false;
	}

	const auto iter = _resources.find(symbol);
	auto stored = iter->second;

	if (stored != resource_) {
		IM_CORE_ERROR("Unexpected stored subject while trying to erase symbolized resource.");
		::hg::breakpoint();
		return false;
	}

	if (stored->data != nullptr) {
		IM_CORE_ERROR("Encountered invalid data state (!= nullptr) while erasing subject.");
		::hg::breakpoint();
		return false;
	}

	_resources.erase(iter);

	const auto* mem = stored.get();
	delete mem;

	return true;
}

bool SymbolContext::eraseImportSymbol(mref<smr<const Symbol>> symbol_) {
	// We are not allowed to manipulate memory of import symbols, cause the associated resources
	//  are owned or proxy owned by other handlers.
	// __noop;
	return true;
}

bool SymbolContext::eraseImportSymbol(mref<nmpt<SymbolizedResource>> resource_) {
	// We are not allowed to manipulate memory of import symbols, cause the associated resources
	//  are owned or proxy owned by other handlers.
	// __noop;
	return true;
}

bool SymbolContext::isShadowing(mref<smr<const Symbol>> symbol_) {

	const bool local = _resources.contains(symbol_);
	auto cur = nmpt<SymbolContext> { this };

	while (local && (cur = cur->parent()) != nullptr) {
		if (cur->_resources.contains(symbol_)) {
			return true;
		}
	}

	return false;
}
