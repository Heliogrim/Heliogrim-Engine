#pragma once

#include <map>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Observed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Symbol.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.GFX/Texture/Texture.hpp>

#include "../SubPass.hpp"

namespace hg::engine::gfx::render {
    class Visualize :
        public SubPass {
    private:
        _STD map<smr<const graph::Symbol>, _STD function<void()>> _available;
        smr<const graph::Symbol> _selected;

        struct Resources {
            graph::Subscribed<graph::SymbolizedResource> outSceneColor;
        } _resources;

    public:
        void destroy() noexcept override;

    public:
        void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

    public:
        void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

        void resolve() noexcept override;

        void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
    };
}
