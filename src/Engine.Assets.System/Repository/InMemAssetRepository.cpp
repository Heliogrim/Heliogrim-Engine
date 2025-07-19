#include "InMemAssetRepository.hpp"

using namespace hg::engine::assets::system;
using namespace hg::engine::assets;
using namespace hg;

InMemAssetRepository::InMemAssetRepository() :
	AssetRepository(),
	_entries() {}

InMemAssetRepository::~InMemAssetRepository() {
	tidy();
}

void InMemAssetRepository::tidy() {
	_entries.clear();
}

bool InMemAssetRepository::destroyAsset(mref<nmpt<Asset>> asset_) {

	const auto key = asset_->getAssetGuid();
	const auto iter = _entries.find(key);

	if (iter == _entries.end()) {
		return false;
	}

	_entries.erase(iter);
	return true;
}

decltype(InMemAssetRepository::_entries)::const_iterator InMemAssetRepository::begin() const noexcept {
	return _entries.cbegin();
}

decltype(InMemAssetRepository::_entries)::const_iterator InMemAssetRepository::end() const noexcept {
	return _entries.cend();
}
