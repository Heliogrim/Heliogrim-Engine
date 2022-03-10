#pragma once

#include "RenderInvocation.hpp"
#include "RenderInvocationData.hpp"
#include "RenderInvocationState.hpp"
#include "RenderSwapchain.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPipeline;
}

namespace ember::engine::gfx {

    class __declspec(novtable) Renderer {
    public:
        using this_type = Renderer;

    protected:
        Renderer() noexcept = default;

    public:
        virtual ~Renderer() = default;

    public:
        virtual void setup(cref<sptr<Device>> device_);

        virtual void destroy() = 0;

    protected:
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    protected:
        [[nodiscard]] virtual const ptr<RenderPipeline> pipeline() const noexcept = 0;

    protected:
        [[nodiscard]] sptr<RenderInvocationState> makeInvocationState() const;

    public:
        virtual ptr<RenderInvocation> allocate(mref<RenderInvocationData> invocation_);

        virtual void free(mref<ptr<RenderInvocation>> invocation_);

    private:
        void invokeBatched(const non_owning_rptr<RenderInvocation> invocation_, mref<CommandBatch> batch_);

    public:
        virtual const non_owning_rptr<RenderInvocation> invoke(const non_owning_rptr<RenderInvocation> invocation_);

        virtual const non_owning_rptr<RenderInvocation> invoke(const non_owning_rptr<RenderInvocation> invocation_,
            cref<CommandBatch> batchLayout_);
    };
}
