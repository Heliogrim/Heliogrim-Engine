#pragma once
#include "NativeBatch.hpp"
#include "Commands/__fwd.hpp"

namespace hg::engine::gfx::render::cmd {
    class RenderCommandBuffer;
}

namespace hg::engine::gfx::render::cmd {
    class __declspec(novtable) RenderCommandTranslator {
    public:
        using this_type = RenderCommandTranslator;

    public:
        constexpr virtual ~RenderCommandTranslator() noexcept = default;

    public:
        [[nodiscard]] virtual ptr<NativeBatch> operator()(const ptr<const RenderCommandBuffer> commands_) noexcept = 0;

    public:
        struct State {
            //
        };

    public:
        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const RenderCommand>) noexcept = 0;

        /**/

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BeginRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BeginAccelerationPassRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BeginSubPassRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const NextSubpassRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const EndSubPassRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const EndAccelerationPassRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const EndRCmd>) noexcept = 0;

        /**/

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindPipelineRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindResourceTableRCmd>) noexcept = 0;

        /**/

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindIndexBufferRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindVertexBufferRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindSkeletalMeshRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindStaticMeshRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindStorageBufferRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const BindTextureRCmd>) noexcept = 0;

        /**/

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const DrawMeshRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const DrawSkeletalMeshRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const DrawDispatchRCmd>) noexcept = 0;

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const DrawDispatchIndirectRCmd>) noexcept = 0;

        /**/

        virtual void translate(_Inout_ ptr<State>, _In_ ptr<const LambdaRCmd>) noexcept = 0;
    };
}
