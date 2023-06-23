#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStage.hpp>
#include <Engine.GFX.Acc/AccelerationInputLayout.hpp>
#include <Engine.GFX.Acc/AccelerationBindingLayout.hpp>
#include <Engine.GFX.Acc/AccelerationOutputLayout.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "Material.hpp"
#include "MaterialPrototype.hpp"
#include "__fwd.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg::engine::gfx::material;
using namespace hg;

void test() {

    auto stage0 = make_smr<AccelerationStage>(
        AccelerationStage {
            nullptr,
            AccelerationStageFlagBits::eVertex,
            {},
            {}
        }
    );

    auto stage1 = make_smr<AccelerationStage>(
        AccelerationStage {
            nullptr,
            AccelerationStageFlagBits::eFragment,
            {},
            {}
        }
    );

    Guid effectGuid {};
    GuidGenerate(effectGuid);

    auto effect = make_smr<AccelerationEffect>(
        AccelerationEffect {
            _STD move(effectGuid),
            "<<name>>",
            { stage0, stage1 },
            {},
            {},
            {}
        }
    );

    /**/

    Guid matProtGuid {};
    GuidGenerate(matProtGuid);

    MaterialPrototype matProt {
        _STD move(matProtGuid),
        "<<name>>",
        { effect },
        {}
    };

    smr<Material> mat0 = matProt.instantiate();
    smr<Material> mat1 = matProt.instantiate();

}
