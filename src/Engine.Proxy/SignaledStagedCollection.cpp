#include "SignaledStagedCollection.hpp"

using namespace ember::engine::proxy;
using namespace ember;

SignaledStagedCollection::SignaledStagedCollection() noexcept = default;

SignaledStagedCollection::SignaledStagedCollection(mref<SignaledStagedCollection> other_) noexcept = default;

SignaledStagedCollection::~SignaledStagedCollection() noexcept = default;

void SignaledStagedCollection::signal(cref<component_type_id> typeId_, cref<sptr<SceneProxiedRef>> proxy_) {
    _signaled.push_back({
        typeId_,
        proxy_
    });
}

decltype(SignaledStagedCollection::_signaled)::iterator SignaledStagedCollection::begin() {
    return _signaled.begin();
}

decltype(SignaledStagedCollection::_signaled)::iterator SignaledStagedCollection::end() {
    return _signaled.end();
}

SignaledStagedCollection SignaledStagedCollection::exchange() {

    SignaledStagedCollection tmp {};
    _STD swap(tmp._signaled, _signaled);

    return tmp;
}
