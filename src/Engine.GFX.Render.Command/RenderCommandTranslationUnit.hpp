#pragma once
#include "RenderCommand.hpp"
#include "RenderCommandTranslationResult.hpp"
#include "Buffer/RenderCommandBufferView.hpp"
#include "Commands/__fwd.hpp"

namespace hg::engine::gfx::render::cmd {
    class __declspec(novtable) RenderCommandTranslationUnit {
    public:
        using this_type = RenderCommandTranslationUnit;

    public:
        constexpr virtual ~RenderCommandTranslationUnit() noexcept = default;

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

        virtual void translate(ptr<BindVertexBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindMaterialRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindSkeletalMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindStaticMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindStorageBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<BindTextureRenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<DrawMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<DrawSkeletalMeshRenderCommand>) noexcept = 0;
    };

    /**/

    class __declspec(novtable) LinearRenderCommandTranslationUnit :
        public RenderCommandTranslationUnit {
    public:
        using this_type = LinearRenderCommandTranslationUnit;

    protected:
        LinearRenderCommandTranslationUnit() noexcept = default;

    public:
        constexpr ~LinearRenderCommandTranslationUnit() noexcept = default;

    public:
        [[nodiscard]] RenderCommandTranslationResult operator()(
            mref<RenderCommandBufferView> commandBufferView_
        ) noexcept override {
            for (const auto* const cmd : commandBufferView_) {
                (*cmd)(this);
            }
            return {};
        }

    public:
        void translate(ptr<RenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BeginRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<NextSubpassRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<EndRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindIndexBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindVertexBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindMaterialRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindSkeletalMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindStaticMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindStorageBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<BindTextureRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<DrawMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<DrawSkeletalMeshRenderCommand>) noexcept override {
            // __noop;
        }
    };
}
