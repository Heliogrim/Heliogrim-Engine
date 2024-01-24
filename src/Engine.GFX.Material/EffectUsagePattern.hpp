#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Render.Predefined/Buffer/ViewBuffer.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

#include "ParameterIdentifier.hpp"

namespace hg::engine::gfx::material {
    /**
     * Used for contextual resolved resources (i.e. camera data)
     */
    struct ContextUsagePattern {
        /**/
        Vector<std::pair<smr<const render::graph::Description>, accel::lang::SymbolId>> pairs;
        // i.e. pair< SceneViewDescription <-> SymbolId >
    };

    /**
     * Used for dynamic material bindings or dynamic substituted resources
     */
    struct DynamicUsagePattern {
        /**/
    };

    /**
     * Used for static material bindings to target effects
     */
    struct StaticUsagePattern {
        Vector<std::pair<ParameterIdentifier, accel::lang::SymbolId>> pairs;
    };

    /**/
    struct EffectUsagePattern {
        ContextUsagePattern ctx;
        DynamicUsagePattern dyn;
        StaticUsagePattern st;
    };
}
