#pragma once

#include "SharedResource.hpp"

namespace hg::engine::resource {
    template <typename ManagedType_>
    class ConcurrentSharedResource final :
        public SharedResource<ManagedType_> {
    public:
        using this_type = ConcurrentSharedResource<ManagedType_>;
        using underlying_type = SharedResource<ManagedType_>;

        using underlying_type::value_type;

    public:
        ConcurrentSharedResource();

        ~ConcurrentSharedResource() override;

    public:
        [[nodiscard]] ResourceBase::loaded_flag_type loaded() const noexcept override;

    public:
        [[nodiscard]] bool try_acquire(ref<ManageGuard> guard_, const ResourceUsageFlags flags_) noexcept override;

        void release(const ResourceUsageFlags flags_) override;
    };
}
