#include "Audio.hpp"

#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Core/Engine.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "Importer/AudioFileTypes.hpp"
#include "Importer/FlacImporter.hpp"
#include "Importer/VorbisImporter.hpp"
#include "Importer/WavImporter.hpp"

using namespace ember::engine;
using namespace ember;

Audio::Audio(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_) {}

Audio::~Audio() noexcept = default;

void Audio::setupImporter() {
    SCOPED_STOPWATCH

    //
    auto* manager { _engine->getResources() };

    //
    manager->importer().registerImporter(sfx::AudioFileType::Flac, new sfx::FlacImporter());
    manager->importer().registerImporter(sfx::AudioFileType::Ogg, new sfx::VorbisImporter());
    manager->importer().registerImporter(sfx::AudioFileType::Wav, new sfx::WavImporter());
}

void Audio::setup() {
    SCOPED_STOPWATCH

    setupImporter();
}

void Audio::schedule() {}

void Audio::desync() {}

void Audio::destroy() {}
