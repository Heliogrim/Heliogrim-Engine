#pragma once

#include <Engine.Accel.Effect/Stage/TransferDataType.hpp>
#include <Engine.Graphics.Material/ParameterIdentifier.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets {
	class GraphicsMaterial final :
		public InheritMeta<GraphicsMaterial, Asset> {
	public:
		using this_type = GraphicsMaterial;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "GraphicsMaterial"_typeId };

	public:
		GraphicsMaterial(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			mref<AssetGuid> prototypeGuid_
		);

	private:
	public:
		struct TmpParam {
			graphics::material::ParameterIdentifier identifier;
			string name;
			accel::TransferDataType dataType;
			Variant<
				AssetGuid,
				u8,
				u16,
				u32,
				u64,
				float,
				math::vec2_t<u8>,
				math::vec2_t<u16>,
				math::vec2_t<u32>,
				math::vec2_t<u64>,
				math::vec2_t<float>,
				math::vec3_t<u8>,
				math::vec3_t<u16>,
				math::vec3_t<u32>,
				math::vec3_t<u64>,
				math::vec3_t<float>,
				math::vec4_t<u8>,
				math::vec4_t<u16>,
				math::vec4_t<u32>,
				math::vec4_t<u64>,
				math::vec4_t<float>,
				math::matq3_t<float>,
				math::matq4_t<float>,
				::std::nullptr_t
			> value;
		};

		AssetGuid _prototype;
		Vector<TmpParam> _params;

	public:
		[[nodiscard]] AssetGuid prototype() const noexcept;

		// TODO: Serialize/Map instantiated parameters
	};
}
