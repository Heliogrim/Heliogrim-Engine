#include "ResourceBase.hpp"

using namespace ember::engine::resource;
using namespace ember;

ResourceBase::ResourceBase() = default;

bool ResourceBase::isLoaded() const noexcept {
    return this->loaded() != 0x0ui8;
}

non_owning_rptr<const EmberObject> ResourceBase::getAssociation() const noexcept {
    return _association;
}

void ResourceBase::setAssociation(const non_owning_rptr<const EmberObject> association_) noexcept {
    _association = association_;
}
