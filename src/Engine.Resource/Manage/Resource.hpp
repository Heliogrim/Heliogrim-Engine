#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Guard.hpp"
#include "../ResourceUsageFlag.hpp"

namespace ember::engine::res {

    // TODO: Make subtypes of resource partial compositions to make explicit comps for every use case
    //  TODO: like: `Streamable<Shared<Resource>>` vs `Streamable<Unique<Resource>>` vs `Streamable<CopyOnWrite<Shared<Resource>>>`
    class __declspec(novtable) Resource {
    public:
        using value_type = Resource;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

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
        virtual ~Resource() = default;

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
         * @returns A ManageGuard&lt;value_type&gt;
         */
        [[nodiscard]] virtual ManageGuard<value_type> acquire(
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) = 0;

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
        [[nodiscard]] virtual bool try_acquire(ptr<ManageGuard<value_type>> guard_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) noexcept = 0;

        /**
         * Releases this resource of use
         *
         * @author Julius
         * @date 28.08.2021
         */
        virtual void release(const ResourceUsageFlags flags_) = 0;
    };

}
