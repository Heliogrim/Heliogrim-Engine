#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>

#include "StagedComponentSubmit.hpp"

namespace ember::engine::ecs::subsystem {

    class StagedComponentQueue {
    public:
        StagedComponentQueue(_In_ component_type_id componentTypeId_) noexcept;

        StagedComponentQueue(mref<StagedComponentQueue> other_) noexcept;

        StagedComponentQueue(cref<StagedComponentQueue>) = delete;

        ~StagedComponentQueue() noexcept;

    public:
        ref<StagedComponentQueue> operator=(mref<StagedComponentQueue> other_) noexcept;

        ref<StagedComponentQueue> operator=(cref<StagedComponentQueue>) = delete;

    private:
        component_type_id _componentTypeId;

    public:
        [[nodiscard]] component_type_id getTypeId() const noexcept;

    private:
        vector<StagedComponentSubmit> _submits;
    };

}
