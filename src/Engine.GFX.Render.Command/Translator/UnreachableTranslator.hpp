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
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BeginRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BeginAccelerationPassRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BeginSubPassRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const NextSubpassRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const EndSubPassRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const EndAccelerationPassRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const EndRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindPipelineRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindResourceTableRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindIndexBufferRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindVertexBufferRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindSkeletalMeshRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindStaticMeshRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindStorageBufferRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const BindTextureRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const DrawMeshRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const DrawSkeletalMeshRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const DrawDispatchRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const DrawDispatchIndirectRCmd>) noexcept override {
            _STD unreachable();
        }

        void translate(ptr<State>, ptr<const LambdaRCmd>) noexcept override {
            _STD unreachable();
        }
    };
}
