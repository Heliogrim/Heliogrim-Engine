#include "VkEffectCompiler.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "Pass/VkPassBuilder.hpp"
#include "Pass/VkPassCompiler.hpp"
#include "Stage/StageComposer.hpp"
#include "Module/VkModuleBuilder.hpp"
#include "Module/VkModuleCompiler.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

uptr<EffectCompiler> hg::engine::gfx::acc::makeVkAccCompiler() {
    return make_uptr<EffectCompiler>(
        make_uptr<VkPassBuilder>(),
        make_uptr<VkPassCompiler>(),
        make_uptr<StageComposer>(),
        make_uptr<VkModuleBuilder>(),
        make_uptr<VkModuleCompiler>(),
        Tokenizer {}
    );
}
