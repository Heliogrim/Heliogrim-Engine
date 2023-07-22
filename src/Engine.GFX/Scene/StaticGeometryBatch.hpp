#pragma once
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "../Cache/ModelBatch.hpp"

#include "../Shader/CachedDiscreteBinding.hpp"
#include "../Buffer/Buffer.hpp"

namespace hg::engine::gfx {
    struct StaticGeometryBatch final :
        public InheritMeta<StaticGeometryBatch, cache::ModelBatch> {
    public:
        inline constexpr static type_id typeId { "StaticGeometryBatch"_typeId };

    public:
        ~StaticGeometryBatch() override;

    public:
        ptr<ModelBatch> clone() override;

        ptr<CachedDiscreteBinding> cdb = nullptr;
        ptr<CachedDiscreteBinding> mttCdb = nullptr;

        Buffer instance { nullptr, nullptr, nullptr, 0ui64, vk::BufferUsageFlags {} };
        Buffer mtt { nullptr, nullptr, nullptr, 0ui64, vk::BufferUsageFlags {} };
    };
}
