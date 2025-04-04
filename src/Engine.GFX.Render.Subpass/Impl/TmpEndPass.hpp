#pragma once

#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/vkinc.hpp>

#include "../SubPass.hpp"

namespace hg::engine::render {
    class TmpEndPass :
        public SubPass {
    public:
        TmpEndPass(vk::ImageLayout targetLayout_);

    private:
        vk::ImageLayout _targetLayout;
        void* _tmpSignal;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
        } _resources;

    public:
        void destroy() noexcept override;

    public:
        void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
