#pragma once

#include <Engine.Accel.Effect/Stage/TransferDataType.hpp>
#include <Engine.Graphics/Geometry/Vertex.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Description.hpp"

namespace hg::engine::render::graph {
	class MeshDescription :
		public InheritMeta<MeshDescription, Description> {
	public:
		using this_type = MeshDescription;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

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
				offsetof(graphics::vertex, position),
				sizeof(graphics::vertex::position),
				accel::TransferDataType::eF32Vec3
			};
			MeshDataLayoutAttribute normal {
				MeshDataLayoutAttributeId::eNormal,
				offsetof(graphics::vertex, normal),
				sizeof(graphics::vertex::normal),
				accel::TransferDataType::eF32Vec3
			};
			MeshDataLayoutAttribute uv {
				MeshDataLayoutAttributeId::eUv,
				offsetof(graphics::vertex, uvm),
				sizeof(graphics::vertex::uvm),
				accel::TransferDataType::eF32Vec3
			};
			MeshDataLayoutAttribute color {
				MeshDataLayoutAttributeId::eColor,
				offsetof(graphics::vertex, color),
				sizeof(graphics::vertex::color),
				accel::TransferDataType::eU8Vec3
			};
			/**/
			u64 stride = sizeof(graphics::vertex);
		};

	private:
		MeshDataLayout _layout;

	public:
		[[nodiscard]] cref<MeshDataLayout> getMeshDataLayout() const noexcept;
	};
}
