#pragma once

#include "Resource.hpp"

namespace ember::engine::resource {
    template <typename ManagedType_>
    class CopyOnWriteResource :
        public Resource<ManagedType_> {
    public:
        using this_type = CopyOnWriteResource<ManagedType_>;
        using underlying_type = Resource<ManagedType_>;

        using underlying_type::value_type;

    public:
        CopyOnWriteResource();

        ~CopyOnWriteResource() override;

    public:
        [[nodiscard]] ResourceBase::loaded_flag_type loaded() const noexcept override;

    public:
        [[nodiscard]] bool try_acquire(ref<ManageGuard> guard_, const ResourceUsageFlags flags_) noexcept override;

        void release(const ResourceUsageFlags flags_) override;
    };
}
