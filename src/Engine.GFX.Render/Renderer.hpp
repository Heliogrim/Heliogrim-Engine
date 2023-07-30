#pragma once
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.GFX/Memory/__fwd.hpp>

#include "InjectGraphRegistry.hpp"

namespace hg::engine::gfx::render {
    class Renderer {
    public:
        using this_type = Renderer;

    protected:
        Renderer() noexcept = default;

    public:
        Renderer(
            mref<Guid> guid_,
            mref<string> name_,
            u32 runtimeVersion_,
            mref<uptr<graph::CompileGraph>> compileGraph_,
            mref<smr<graph::Resolver>> baseResolver_,
            mref<smr<graph::InjectGraphRegistry>> injectionRegistry_,
            /**/
            mref<smr<cache::GlobalCacheCtrl>> globalCache_,
            mref<smr<memory::GlobalPooledAllocator>> globalGfxAllocator_
        ) noexcept;

        virtual ~Renderer() = default;

    private:
        Guid _guid;
        string _name;

        _STD atomic_uint_fast32_t _rtVer;

        uptr<graph::CompileGraph> _compileGraph;
        smr<graph::Resolver> _baseRes;

        smr<graph::InjectGraphRegistry> _injectReg;

    private:
        smr<cache::GlobalCacheCtrl> _globalCache;
        smr<memory::GlobalPooledAllocator> _globalGfxAlloc;

    private:
        [[nodiscard]] virtual smr<graph::Resolver> makeAdvancedResolver() const;

        [[nodiscard]] virtual uptr<RenderPipeline> makeRenderPipeline() const;

        [[nodiscard]] virtual uptr<pipeline::State> makePipelineState(
            mref<nmpt<const RenderPipeline>> pipeline_
        ) const;

        [[nodiscard]] virtual uptr<pipeline::State> updatePipelineState(
            mref<nmpt<const RenderPipeline>> pipeline_,
            mref<uptr<pipeline::State>> state_
        ) const;

    public:
        [[nodiscard]] _Success_(return != nullptr) virtual uptr<class RenderPass> allocate() const;

        [[nodiscard]] virtual _STD pair<uptr<RenderPass>, bool> reallocate(
            mref<uptr<RenderPass>> pass_
        ) const;

        virtual void free(mref<uptr<RenderPass>> pass_) const;

    private:
        [[nodiscard]] virtual uptr<RenderPass> updateIncremental(
            mref<uptr<RenderPass>> pass_,
            mref<uptr<graph::RuntimeGraph>> nextGraph_
        ) const;

    public:
        [[nodiscard]] virtual uptr<RenderPass> updateOnDelta(mref<uptr<RenderPass>> pass_) const;
    };
}
