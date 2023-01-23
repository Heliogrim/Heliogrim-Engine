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

    private:
        ptr<value_type> _value;

    public:
        loaded_flag_type loaded() const noexcept override {
            return (loaded_flag_type)(_value != nullptr);
        }

    protected:
        [[nodiscard]] const non_owning_rptr<void> value() const noexcept override {
            return _value;
        }

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
        template <typename Type_ = ManagedType_> requires _STD is_same_v<Type_, ManagedType_>
        [[nodiscard]] TypedManageGuard<Type_> acquire(
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) {
            return static_cast<ptr<ResourceBase>>(this)->acquire(flags_);
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
        template <typename Type_ = ManagedType_> requires _STD is_same_v<Type_, ManagedType_>
        [[nodiscard]] bool try_acquire(
            _Out_ ref<TypedManageGuard<Type_>> guard_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) noexcept {
            return static_cast<ptr<ResourceBase>>(this)->try_acquire(static_cast<ref<ManageGuard>>(guard_), flags_);
        }
    };
}
