#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Audio/AudioFormat.hpp>

#include "../Asset.hpp"

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
