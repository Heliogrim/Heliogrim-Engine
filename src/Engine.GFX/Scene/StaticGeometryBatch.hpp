#pragma once
#include "../Cache/ModelBatch.hpp"

#include "../Shader/CachedDiscreteBinding.hpp"
#include "../Buffer/Buffer.hpp"

namespace ember::engine::gfx {

    struct StaticGeometryBatch final :
        public cache::ModelBatch {
    public:
        inline constexpr static type_id typeId { "StaticGeometryBatch"_typeId };

    public:
        ~StaticGeometryBatch() override;

    public:
        ptr<ModelBatch> clone() override;

        ptr<CachedDiscreteBinding> cdb = nullptr;
        Buffer instance { nullptr, nullptr, nullptr, 0ui64, vk::BufferUsageFlags {} };
    };

}
