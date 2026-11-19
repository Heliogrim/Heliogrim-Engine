#pragma once

#include <Engine.Audio/AudioFormat.hpp>
#include <Engine.Common/__macro.hpp>

#include "../Asset.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets {
	class Sound :
		public InheritMeta<Sound, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "Sound"_typeId };

	protected:
		Sound(mref<AssetGuid> guid_);

	private:
		string _url;

	private:
		u8 _channel;

	private:
		audio::AudioFormat _format;

	private:
		u32 _sampleRate;

	private:
		u64 _samples;
	};
}
