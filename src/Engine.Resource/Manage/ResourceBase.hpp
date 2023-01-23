#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/Types/Asset.hpp>

#include "../ResourceUsageFlag.hpp"
#include "ManageGuard.hpp"

namespace ember::engine::resource {
    class __declspec(novtable) ResourceBase {
    public:
        using this_type = ResourceBase;

        using loaded_flag_type = u8;

    protected:
        ResourceBase();

    public:
        virtual ~ResourceBase() = default;

    public:
        /**
         * Get the current loading state of this resource
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @returns The loaded_flag_type value.
         */
        [[nodiscard]] virtual loaded_flag_type loaded() const noexcept = 0;

        /**
         * Check whether this is loaded
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @returns True if loaded, false if not.
         */
        [[nodiscard]] bool isLoaded() const noexcept;

    public:
        /**
         * Acquire this resource to use
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  flags_ (Optional) The flags.
         *
         * @returns A ManageGuard instance.
         */
        [[nodiscard]] virtual ManageGuard acquire(
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) = 0;

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
        [[nodiscard]] virtual bool try_acquire(
            _Inout_ ref<ManageGuard> guard_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) noexcept = 0;

        /**
         * Releases this resource of use
         *
         * @author Julius
         * @date 28.08.2021
         */
        virtual void release(const ResourceUsageFlags flags_) = 0;
    };
}
