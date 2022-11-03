#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.SFX/AudioFormat.hpp>

#include "Asset.hpp"

namespace ember::engine::assets {
    class Sound :
        public Asset {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "Sound"_typeId };

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
