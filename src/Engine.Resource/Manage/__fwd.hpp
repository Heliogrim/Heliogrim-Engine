#pragma once

namespace hg::engine::resource {
    template <typename>
    class Resource;
    class ResourceBase;

    template <typename>
    class TypedManageGuard;
    class ManageGuard;

    template <typename>
    class ConcurrentSharedResource;

    template <typename>
    class CopyOnWriteResource;

    template <typename>
    class MutexSharedResource;

    template <typename>
    class SharedResource;

    template <typename>
    class UniqueResource;
}
