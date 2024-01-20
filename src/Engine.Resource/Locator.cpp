#include "Locator.hpp"

using namespace hg::engine::resource;
using namespace hg;

Locator::reference_type Locator::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _mapping = _STD move(other_._mapping);
    }

    return *this;
}

ptr<Source> Locator::get(cref<asset_guid> guid_) const noexcept {
    const auto entry = _mapping.find({ guid_ });
    if (entry != _mapping.cend()) {
        return entry->second;
    }
    return nullptr;
}

ptr<Source> Locator::insert(cref<asset_guid> guid_, ptr<Source> source_) noexcept {
    const auto entry = static_cast<const map_type>(_mapping).find({ guid_ });
    auto* const prev = entry != _mapping.cend() ? entry->second : nullptr;
    _mapping.insert_or_assign(guid_, source_);
    return prev;
}
