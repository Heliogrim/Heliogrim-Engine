#pragma once

#include <Engine.Accel.Effect/Stage/TransferDataType.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX.Material/ParameterIdentifier.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

namespace hg::engine::assets {
	class GfxMaterial final :
		public InheritMeta<GfxMaterial, Asset> {
	public:
		using this_type = GfxMaterial;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "GfxMaterial"_typeId };

	public:
		GfxMaterial(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			mref<AssetGuid> prototypeGuid_
		);

	private:
	public:
		struct TmpParam {
			gfx::material::ParameterIdentifier identifier;
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
