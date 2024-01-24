#pragma once
#include "../RenderCommandTranslator.hpp"

namespace hg::engine::render::cmd {
    class __declspec(novtable) UnreachableCmdTl :
        public RenderCommandTranslator {
    public:
        using this_type = UnreachableCmdTl;

    protected:
        UnreachableCmdTl() noexcept = default;

    public:
        constexpr ~UnreachableCmdTl() noexcept override = default;

    public:
        void translate(ptr<State>, ptr<const RenderCommand>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BeginRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BeginAccelerationPassRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BeginSubPassRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const NextSubpassRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const EndSubPassRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const EndAccelerationPassRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const EndRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindPipelineRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindResourceTableRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindIndexBufferRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindVertexBufferRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindSkeletalMeshRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindStaticMeshRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindStorageBufferRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const BindTextureRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const DrawMeshRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const DrawSkeletalMeshRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const DrawDispatchRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const DrawDispatchIndirectRCmd>) noexcept override {
            std::unreachable();
        }

        void translate(ptr<State>, ptr<const LambdaRCmd>) noexcept override {
            std::unreachable();
        }
    };
}
