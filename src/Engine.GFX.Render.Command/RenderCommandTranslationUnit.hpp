#pragma once
#include "Buffer/RenderCommandBufferView.hpp"
#include "Commands/__fwd.hpp"

namespace hg::engine::gfx::render::cmd {
    class RenderCommand;
    class RenderCommandTranslationResult;
}

namespace hg::engine::gfx::render::cmd {
    class RenderCommandTranslationUnit {
    public:
        using this_type = RenderCommandTranslationUnit;

    public:
        [[nodiscard]] virtual RenderCommandTranslationResult operator()(
            mref<RenderCommandBufferView> commandBufferView_
        ) noexcept = 0;

    public:
        virtual void translate(ptr<RenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<BeginRenderCommand>) noexcept = 0;

        virtual void translate(ptr<NextSubpassRenderCommand>) noexcept = 0;

        virtual void translate(ptr<EndRenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<BindIndexBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindMaterialRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindSkeletalMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindStaticMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindStorageBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindTextureRenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<DrawMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<DrawSkeletalMeshRenderCommand>) noexcept = 0;
    };
}
