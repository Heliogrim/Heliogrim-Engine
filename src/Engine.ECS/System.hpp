#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/List.hpp>

#include "ComponentTypeId.hpp"
#include "Registry.hpp"
#include "SystemComponentReflection.hpp"
#include "Traits.hpp"

namespace ember::engine::ecs {

    class System {
    public:
        using this_type = System;

        using pool_size_type = u64;
        using pool_entry_size_type = u64;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 30.10.2021
         */
        System() noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 30.10.2021
         */
        System(cref<System>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 30.10.2021
         */
        System(mref<System>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.10.2021
         */
        ~System() noexcept;

    public:
        /**
         * Copy Assignment Operator
         *
         * @author Julius
         * @date 30.10.2021
         */
        ref<System> operator=(cref<System>) = delete;

        /**
         * Move Assignment Operator
         *
         * @author Julius
         * @date 30.10.2021
         */
        ref<System> operator=(mref<System>) noexcept = delete;

    private:
        uptr<Registry> _registry;

    public:
        [[nodiscard]] non_owning_rptr<Registry> registry() const noexcept;

    private:
        ska::bytell_hash_map<component_type_id, SystemComponentReflection> _reflections;

    public:
        [[nodiscard]] ptr<void> getComponent(_In_ cref<component_type_id> typeId_,
            _In_ cref<entity_guid> entity_) const noexcept;

    public:
        void makeComponent(_In_ cref<component_type_id> typeId_) = delete;

        void makeComponent(_In_ cref<component_type_id> typeId_, _In_ const ptr<void> source_) = delete;

        void makeComponents(_In_ cref<component_type_id> typeId_, _In_ cref<pool_entry_size_type> count_) = delete;

        void mantleComponent(_In_ cref<component_type_id> typeId_, _In_ cref<entity_guid> entity_);

        void mantleComponents(_In_ cref<component_type_id> typeId_, _In_ cref<vector<entity_guid>> entities_);

        void dismantleComponent(_In_ cref<component_type_id> typeId_, _In_ cref<entity_guid> entity_) = delete;

        void dismantleComponents(_In_ cref<component_type_id> typeId_, _In_ cref<vector<entity_guid>> entities_)
        = delete;

        void destroyComponent(_In_ cref<component_type_id> typeId_, _In_ cref<entity_guid> entity_);

        void destroyComponents(_In_ cref<component_type_id> typeId_, _In_ cref<vector<entity_guid>> entities_);

        void destroyAllComponents(_In_ cref<component_type_id> typeId_);

    public:
        void compose(_In_ cref<entity_guid> guid_, _STD initializer_list<component_type_id> typeIds_);

        void compose(_In_ cref<entity_guid> guid_,
            _STD initializer_list<_STD pair<component_type_id, ptr<void>>> typeSrcPairs_);

    public:
        template <IsComponent ComponentType_>
        void registerComponent() {
            [[maybe_unused]] auto& pool = _registry->getOrCreatePool<ComponentType_>();

            // TODO: Rework
            // Warning: Function pointer of constructor/destructor is unavailable
            auto refl = SystemComponentReflection {
                ComponentType_::type_id,
                sizeof(ComponentType_),
                nullptr/* &ComponentType_::ComponentType_ */,
                nullptr/* &ComponentType_::mantle */,
                nullptr/* &ComponentType_::dismantle */,
                nullptr/* &ComponentType_::~ComponentType_ */
            };

            if constexpr (IsMantleableComponent<ComponentType_>) {
                refl.mantle = reinterpret_cast<SystemComponentReflection::mantle_fnc_type>(&ComponentType_::mantle);
            }

            if constexpr (IsDismantleableComponent<ComponentType_>) {
                refl.dismantle = reinterpret_cast<SystemComponentReflection::dismantle_fnc_type>(&ComponentType_::dismantle);
            }

            _reflections.insert(_STD make_pair(ComponentType_::type_id, refl));
        }
    };
}
