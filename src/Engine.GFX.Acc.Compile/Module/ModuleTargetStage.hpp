#pragma once

namespace hg::engine::gfx::acc {
    enum class ModuleTargetStage {
        eUndefined,
        /**/
        eVertex = 0x1,
        eTessCtrl = 0x2,
        eTessEval = 0x3,
        eGeometry = 0x4,
        eFragment = 0x5,
        /**/
        eCompute = 0x6,
        /**/
        eRayGen = 0x7,
        eRayIntersect = 0x8,
        eRayHitAny = 0x9,
        eRayHitClosest = 0xA,
        eRayMiss = 0xB,
        /**/
        eTask = 0xD,
        eMesh = 0xE,
        eMeshFrag = eFragment
    };
}
