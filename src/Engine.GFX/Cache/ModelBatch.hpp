#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/HeliogrimReflect.hpp>

namespace hg::engine::gfx::cache {
    // Warning: Do NOT instantiate ModelBatch itself (it might fail anyway cause of `novtable` annotation)
    struct __declspec(novtable) ModelBatch :
        public HeliogrimObject {
    public:
        virtual ~ModelBatch() = default;

    public:
        ref<ModelBatch> operator=(cref<ModelBatch>) = delete;

        ref<ModelBatch> operator=(mref<ModelBatch>) noexcept = default;

    public:
        virtual ptr<ModelBatch> clone() = 0;
    };
}
