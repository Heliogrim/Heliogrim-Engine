#include "DxEffectCompiler.hpp"

#include <Engine.Common/Make.hpp>

#include "Pass/DxPassBuilder.hpp"
#include "Pass/DxPassCompiler.hpp"
#include "Stage/StageComposer.hpp"
#include "Module/DxModuleBuilder.hpp"
#include "Module/DxModuleCompiler.hpp"

using namespace hg::engine::accel;
using namespace hg;

uptr<EffectCompiler> engine::accel::makeDxAccCompiler() {
    return make_uptr<EffectCompiler>(
        make_uptr<DxPassBuilder>(),
        make_uptr<DxPassCompiler>(),
        make_uptr<StageComposer>(),
        make_uptr<DxModuleBuilder>(),
        make_uptr<DxModuleCompiler>()
    );
}
