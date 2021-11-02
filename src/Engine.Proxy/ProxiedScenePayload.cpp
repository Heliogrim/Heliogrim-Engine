#include "ProxiedScenePayload.hpp"

using namespace ember::engine::proxy;
using namespace ember;

ProxiedScenePayload::ProxiedScenePayload(mref<payload_type> payload_) noexcept :
    _payload(_STD move(payload_)) {}

ProxiedScenePayload::ProxiedScenePayload(cref<OwningProxiedScenePayload> parent_) noexcept :
    _payload(parent_.payload()) {}

ProxiedScenePayload::ProxiedScenePayload(mref<ProxiedScenePayload> other_) noexcept :
    _payload(_STD move(other_._payload)) {}

ref<ProxiedScenePayload> ProxiedScenePayload::operator=(mref<ProxiedScenePayload> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _payload = _STD move(other_._payload);
    }

    return *this;
}

ref<ProxiedScenePayload> ProxiedScenePayload::operator=(cref<OwningProxiedScenePayload> parent_) noexcept {

    if (_STD addressof(parent_) != static_cast<const void*>(this)) {
        _payload = parent_.payload();
    }

    return *this;
}

cref<ProxiedScenePayload::payload_type> ProxiedScenePayload::payload() const noexcept {
    return _payload;
}

ref<ProxiedScenePayload::payload_type> ProxiedScenePayload::payload() noexcept {
    return _payload;
}

OwningProxiedScenePayload::OwningProxiedScenePayload(mref<payload_type> payload_) noexcept :
    _payload(_STD move(payload_)) {}

OwningProxiedScenePayload::OwningProxiedScenePayload(mref<OwningProxiedScenePayload> other_) noexcept :
    _payload(_STD move(other_._payload)) {}

ref<OwningProxiedScenePayload> OwningProxiedScenePayload::operator=(mref<OwningProxiedScenePayload> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _payload = _STD move(other_._payload);
    }

    return *this;
}

cref<OwningProxiedScenePayload::payload_type> OwningProxiedScenePayload::payload() const noexcept {
    return _payload;
}

ref<OwningProxiedScenePayload::payload_type> OwningProxiedScenePayload::payload() noexcept {
    return _payload;
}
