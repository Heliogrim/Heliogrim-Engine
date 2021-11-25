#include "ProxyTypeRegister.hpp"

using namespace ember::engine::proxy;
using namespace ember;

ProxyTypeRegister::ProxyTypeRegister() noexcept = default;

ProxyTypeRegister::~ProxyTypeRegister() noexcept = default;

void ProxyTypeRegister::add(cref<component_type_id> typeId_, cref<transit_fnc_type> fnc_) {
    _mapped.insert_or_assign(typeId_, fnc_);
}

ProxyTypeRegister::transit_fnc_type ProxyTypeRegister::get(cref<component_type_id> typeId_) const noexcept {
    const auto entry = _mapped.find(typeId_);

    if (entry != _mapped.end()) {
        return entry->second;
    }

    return nullptr;
}
