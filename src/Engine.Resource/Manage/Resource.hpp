#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/Types/Asset.hpp>

#include "../ResourceUsageFlag.hpp"
#include "ResourceBase.hpp"
#include "TypedManageGuard.hpp"

namespace ember::engine::resource {
    template <typename ManagedType_>
    class __declspec(novtable) Resource :
        public ResourceBase {
    public:
        using this_type = Resource;

        using value_type = ManagedType_;
        using loaded_flag_type = u8;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 27.08.2021
         */
        Resource() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 27.08.2021
         */
        ~Resource() override = default;

    public:
        /**
         * Acquire this resource to use
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  flags_ (Optional) The flags.
         *
         * @returns A TypedManageGuard instance.
         */
        [[nodiscard]] TypedManageGuard<value_type> acquire(
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) {
            return this->acquire(flags_);
        }

        /**
         * Attempts to acquire this resource to use
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  guard_ The guard.
         * @param  flags_ (Optional) The flags.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool try_acquire(
            _Out_ ref<TypedManageGuard<value_type>> guard_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) noexcept {
            return this->try_acquire(static_cast<ref<ManageGuard>>(guard_), flags_);
        }
    };
}
