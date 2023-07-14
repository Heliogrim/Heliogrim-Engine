#pragma once

#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    /**
     * Plane mesh invocation generator is a specialization of a mesh invocation generator used to dispatch
     *  passes and materials with a plane capturing the complete framebuffer image. This is primarily used
     *  for post-processing, composing or e.g. deferred lighting computation.
     */
    class PlaneMeshIG :
        public InvocationGenerator {
    public:
    };
}
