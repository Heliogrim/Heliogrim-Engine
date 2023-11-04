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
        virtual void translate(ptr<const RenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<const BeginRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const NextSubpassRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const EndRenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<const BindIndexBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindVertexBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindMaterialRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindSkeletalMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindStaticMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindStorageBufferRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const BindTextureRenderCommand>) noexcept = 0;

        /**/

        virtual void translate(ptr<const DrawMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const DrawSkeletalMeshRenderCommand>) noexcept = 0;

        virtual void translate(ptr<const DrawDispatchRenderCommand>) noexcept = 0;
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
        void translate(ptr<const RenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BeginRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const NextSubpassRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const EndRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindIndexBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindVertexBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindMaterialRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindSkeletalMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindStaticMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindStorageBufferRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const BindTextureRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const DrawMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const DrawSkeletalMeshRenderCommand>) noexcept override {
            // __noop;
        }

        void translate(ptr<const DrawDispatchRenderCommand>) noexcept override {
            // __noop;
        }
    };
}
