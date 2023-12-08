#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

namespace hg::engine::accel {
    /**
     * Used for contextual resolved resources (i.e. camera data)
     */
    struct ContextUsagePattern {
        /**/
        Vector<_STD pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
        // i.e. pair< SceneViewDescription <-> SymbolId >
    };

    struct DynamicUsagePattern {
        /**/
        Vector<_STD pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
        // i.e. pair< MeshDescription <-> SymbolId >
    };

    struct StaticUsagePattern {
        // Note: Currently unsupported
    };

    /**/
    struct ProfileUsagePattern {
        ContextUsagePattern ctx;
        DynamicUsagePattern dyn;
        StaticUsagePattern st;
    };
}
