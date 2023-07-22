#include "Audio.hpp"

#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Core/Engine.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Make.hpp>

#include "Importer/AudioFileTypes.hpp"
#include "Importer/FlacImporter.hpp"
#include "Importer/VorbisImporter.hpp"
#include "Importer/WavImporter.hpp"

using namespace hg::engine;
using namespace hg;

Audio::Audio(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_) {}

Audio::~Audio() noexcept = default;

void Audio::setup() {
    SCOPED_STOPWATCH

    registerImporter();
}

void Audio::start() {}

void Audio::stop() {}

void Audio::destroy() {
    unregisterImporter();
}

void Audio::registerImporter() {

    auto* manager { _engine->getResources() };

    //
    manager->importer().registerImporter(sfx::AudioFileType::Flac, make_sptr<sfx::FlacImporter>());
    manager->importer().registerImporter(sfx::AudioFileType::Ogg, make_sptr<sfx::VorbisImporter>());
    manager->importer().registerImporter(sfx::AudioFileType::Wav, make_sptr<sfx::WavImporter>());
}

void Audio::unregisterImporter() {

    auto* manager { _engine->getResources() };

    /**/
    manager->importer().unregisterImporter(sfx::AudioFileType::Flac);
    manager->importer().unregisterImporter(sfx::AudioFileType::Ogg);
    manager->importer().unregisterImporter(sfx::AudioFileType::Wav);
}
