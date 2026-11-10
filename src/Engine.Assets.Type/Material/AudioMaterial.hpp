#pragma once
#include "Material.hpp"

namespace hg::engine::assets {
	class AudioMaterial :
		public InheritMeta<AudioMaterial, Material> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "AudioMaterial"_typeId };
	};
}
