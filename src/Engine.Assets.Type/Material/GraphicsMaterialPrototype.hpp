#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.Types;
#endif

namespace hg::engine::assets {
	class GraphicsMaterialPrototype :
		public InheritMeta<GraphicsMaterialPrototype, Asset> {
	public:
		using this_type = GraphicsMaterialPrototype;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "GraphicsMaterialPrototype"_typeId };

	public:
		GraphicsMaterialPrototype(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_
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
			> defaultValue;
		};

		// TODO: Serialize/Map instantiated parameters
		Vector<AssetGuid> _effects;
		Vector<TmpParam> _params;
	};
}
