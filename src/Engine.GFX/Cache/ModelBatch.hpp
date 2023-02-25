#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/EmberReflect.hpp>

namespace ember::engine::gfx::cache {
    // Warning: Do NOT instantiate ModelBatch itself (it might fail anyway cause of `novtable` annotation)
    struct __declspec(novtable) ModelBatch :
        public EmberObject {
    public:
        virtual ~ModelBatch() = default;

    public:
        ref<ModelBatch> operator=(cref<ModelBatch>) = delete;

        ref<ModelBatch> operator=(mref<ModelBatch>) noexcept = default;

    public:
        virtual ptr<ModelBatch> clone() = 0;
    };
}
