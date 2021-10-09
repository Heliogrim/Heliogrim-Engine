#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.SFX/AudioFormat.hpp>

#include "Data.hpp"
#include "SoundLayout.hpp"

namespace ember::engine::assets {
    class Sound :
        public Data<SoundLayout> {
    public:
        inline static const asset_type_id type_id { "Sound"_typeId };

    protected:
        Sound(cref<asset_guid> guid_);

    private:
        string _url;

    private:
        u8 _channel;

    private:
        sfx::AudioFormat _format;

    private:
        u32 _sampleRate;

    private:
        u64 _samples;
    };
}
