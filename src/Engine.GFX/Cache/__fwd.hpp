#pragma once

#include <concepts>

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx::cache {

    class GlobalCacheCtrl;
    class LocalCacheCtrl;
    class GlobalResourceCache;
    class LocalResourceCache;
    class TransientCache;

    struct ModelBatch;

    enum CacheResult: u8;

    template <std::totally_ordered SubjectType_>
    class CacheCtrlSubject;

    struct CacheStaticGeometrySubject;
    struct CacheTextureSubject;
}
