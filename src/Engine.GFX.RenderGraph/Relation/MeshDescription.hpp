#pragma once

#include <Engine.Accel.Effect/Stage/TransferDataType.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/Geometry/Vertex.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    class MeshDescription :
        public InheritMeta<MeshDescription, Description> {
    public:
        using this_type = MeshDescription;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        MeshDescription() noexcept;

        ~MeshDescription() override;

    protected:
        [[nodiscard]] bool isValueCompatible(
            const non_owning_rptr<const Description> other_
        ) const noexcept override;

    public:
        enum class MeshDataLayoutAttributeId {
            ePosition,
            eNormal,
            eUv,
            eColor
        };

        struct MeshDataLayoutAttribute {
            MeshDataLayoutAttributeId id;
            ptrdiff_t offset { 0LL };
            ptrdiff_t size { 0LL };
            /**/
            accel::TransferDataType dataType;
        };

        struct MeshDataLayout {
            MeshDataLayoutAttribute position {
                MeshDataLayoutAttributeId::ePosition,
                offsetof(vertex, position),
                sizeof(vertex::position),
                accel::TransferDataType::eF32Vec3
            };
            MeshDataLayoutAttribute normal {
                MeshDataLayoutAttributeId::eNormal,
                offsetof(vertex, normal),
                sizeof(vertex::normal),
                accel::TransferDataType::eF32Vec3
            };
            MeshDataLayoutAttribute uv {
                MeshDataLayoutAttributeId::eUv,
                offsetof(vertex, uvm),
                sizeof(vertex::uvm),
                accel::TransferDataType::eF32Vec3
            };
            MeshDataLayoutAttribute color {
                MeshDataLayoutAttributeId::eColor,
                offsetof(vertex, color),
                sizeof(vertex::color),
                accel::TransferDataType::eU8Vec3
            };
            /**/
            u64 stride = sizeof(vertex);
        };

    private:
        MeshDataLayout _layout;

    public:
        [[nodiscard]] cref<MeshDataLayout> getMeshDataLayout() const noexcept;
    };
}
