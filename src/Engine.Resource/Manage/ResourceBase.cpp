#include "ResourceBase.hpp"

using namespace hg::engine::resource;
using namespace hg;

ResourceBase::ResourceBase() noexcept = default;

bool ResourceBase::isLoaded() const noexcept {
    return this->loaded() != 0x0ui8;
}

non_owning_rptr<const HeliogrimObject> ResourceBase::getAssociation() const noexcept {
    return _association;
}

void ResourceBase::setAssociation(const non_owning_rptr<const HeliogrimObject> association_) noexcept {
    _association = association_;
}
