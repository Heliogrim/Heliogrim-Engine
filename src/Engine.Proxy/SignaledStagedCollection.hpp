#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.ACS/ComponentTypeId.hpp>

#include "SceneProxiedRef.hpp"

namespace ember::engine::proxy {

    struct SignaledEntry {
        component_type_id typeId;
        sptr<SceneProxiedRef> proxied;
    };

    class SignaledStagedCollection {
    public:
        using this_type = SignaledStagedCollection;

    public:
        SignaledStagedCollection() noexcept;

        SignaledStagedCollection(cref<SignaledStagedCollection>) = delete;

        SignaledStagedCollection(mref<SignaledStagedCollection> other_) noexcept;

        ~SignaledStagedCollection() noexcept;

    private:
        Vector<SignaledEntry> _signaled;

    public:
        void signal(cref<component_type_id> typeId_, cref<sptr<SceneProxiedRef>> proxy_);

        [[nodiscard]] decltype(_signaled)::iterator begin();

        [[nodiscard]] decltype(_signaled)::iterator end();

    public:
        [[nodiscard]] SignaledStagedCollection exchange();
    };
}
