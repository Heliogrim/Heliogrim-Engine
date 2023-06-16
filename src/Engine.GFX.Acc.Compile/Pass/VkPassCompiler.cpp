#include "VkPassCompiler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Acc/Pass/VkAccelerationGraphicsPass.hpp>

#include "../Module/VkCompiledModule.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

VkPassCompiler::VkPassCompiler() = default;

VkPassCompiler::~VkPassCompiler() = default;

smr<const AccelerationPass> VkPassCompiler::compile(
    mref<smr<AccelerationPass>> source_,
    mref<Vector<uptr<CompiledModule>>> modules_
) const {

    auto obj = source_.into<VkAccelerationGraphicsPass>();
    const auto& effect = *(obj->getEffect().get());

    /* Link stage, compiled modules and derivates */

    const auto size = effect.getStages().size();
    for (size_t idx = 0; idx < size; ++idx) {

        const auto& srcStage = effect.getStages()[idx];
        auto compModule = _STD move(modules_[idx]);

        /* Build acceleration stage module from compiler module */

        auto accModule = make_smr<AccelerationStageModule>();

        /* Compose stage derivat out of src stage and modules */

        auto derivat = make_smr<AccelerationStageDerivat>(
            srcStage.get(),
            Vector<smr<AccelerationStage>> { srcStage },
            _STD move(accModule),
            AccelerationStageFlags {},
            Vector<AccelerationStageInput> { srcStage->getStageInputs() },
            Vector<AccelerationStageOutput> { srcStage->getStageOutputs() },
            Vector<AccelerationStageInput> {},
            Vector<AccelerationStageOutput> {}
        );

        obj->setStageDerivat(idx, _STD move(derivat));
    }

    /**/

    {
        sptr<Device> device = Engine::getEngine()->getGraphics()->getCurrentDevice();

        vk::PipelineCache vkPipeCache {};
        vk::PipelineLayout vkPipeLayout {};
        vk::RenderPass vkRenderPass {};

        vk::GraphicsPipelineCreateInfo gpci {
            vk::PipelineCreateFlags {},
            static_cast<u32>(obj->getStageDerivates().size()),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            vkPipeLayout,
            vkRenderPass,
            NULL,
            VK_NULL_HANDLE,
            -1i32
        };

        const auto result = device->vkDevice().createGraphicsPipeline(vkPipeCache, gpci);
        obj->setVkPipe(reinterpret_cast<_::VkGraphicsPipeline>(result.value.operator VkPipeline()));
    }

    return obj;
}
