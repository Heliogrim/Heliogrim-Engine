#include "VkEffectCompiler.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "Pass/VkPassBuilder.hpp"
#include "Pass/VkPipelineCompiler.hpp"
#include "Stage/StageComposer.hpp"
#include "Module/VkModuleBuilder.hpp"
#include "Module/VkModuleCompiler.hpp"

using namespace hg::engine::accel;
using namespace hg;

uptr<EffectCompiler> hg::engine::accel::makeVkAccCompiler() {
    return make_uptr<EffectCompiler>(
        make_uptr<VkPassBuilder>(),
        make_uptr<VkPipelineCompiler>(),
        make_uptr<StageComposer>(),
        make_uptr<VkModuleBuilder>(),
        make_uptr<VkModuleCompiler>()
    );
}
