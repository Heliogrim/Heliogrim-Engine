#include "Audio.hpp"

#include <Engine.Resource/ResourceManager.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Session/Session.hpp>

#include "Importer/AudioFileTypes.hpp"
#include "Importer/FlacImporter.hpp"
#include "Importer/VorbisImporter.hpp"
#include "Importer/WavImporter.hpp"

using namespace ember::engine;
using namespace ember;

Audio::Audio(cref<sptr<Session>> session_) noexcept :
    _session(session_) {}

Audio::~Audio() noexcept {}

void Audio::setupImporter() {

    SCOPED_STOPWATCH

    //
    auto* manager { _session->modules().resourceManager() };

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

sptr<Session> Audio::session() const noexcept {
    return _session;
}
