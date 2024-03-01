#include "WavImporter.hpp"

#include <Engine.Common/Buffer.hpp>
#include <Engine.Common/Make.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#define DR_WAV_IMPLEMENTATION
#include <dr_wav/dr_wav.hpp>

#include "AudioFileTypes.hpp"

using namespace hg::engine::sfx;
using namespace hg;

bool WavImporter::canImport(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const noexcept {
    return typeId_ == AudioFileType::Wav;
}

WavImporter::descriptor_type WavImporter::descriptor() const noexcept {
    return {};
}

WavImporter::import_result_type WavImporter::import(cref<res::FileTypeId> typeId_, cref<fs::File> file_) const {

    SCOPED_STOPWATCH

    drwav wav;
    if (!drwav_init_file(&wav, file_.path().string().c_str())) {
        // Error opening
        throw std::runtime_error("Could not open file to parse wav data.");
    }

    Buffer buffer {
        .size = wav.totalPCMFrameCount * wav.channels * sizeof(float),
        .align = 0uLL,
        .mem = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(float))
    };

    u64 decodedSamples = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, static_cast<ptr<float>>(buffer.mem));

    import_type result {
        .channels = static_cast<u8>(wav.channels),
        .format = AudioFormat::eF32,
        .samples = decodedSamples,
        .sampleRate = wav.sampleRate,
        .buffer = buffer
    };

    drwav_uninit(&wav);

    //
    auto state { make_sptr<concurrent::future_state<import_type>>() };
    state->set(std::move(result));

    return import_result_type { state };
}
