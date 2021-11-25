#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.ACS/ComponentTypeId.hpp>

#include "SceneProxiedRef.hpp"

namespace ember::engine::proxy {

    class ProxyTypeRegister {

    public:
        using this_type = ProxyTypeRegister;

        using transit_fnc_type = _STD function<void(ptr<SceneProxiedRef>)>;
    public:
        ProxyTypeRegister() noexcept;

        ~ProxyTypeRegister() noexcept;

    private:
        ska::bytell_hash_map<component_type_id, transit_fnc_type> _mapped;

    public:
        void add(cref<component_type_id> typeId_, cref<transit_fnc_type> fnc_);

        void add(cref<component_type_id> typeId_, const ptr<void> self_, const ptr<void> fnc_);

        void remove(cref<component_type_id> typeId_, cref<transit_fnc_type> fnc_) = delete;

        [[nodiscard]] transit_fnc_type get(cref<component_type_id> typeId_) const noexcept;
    };
}
