#include "Audio.hpp"

#include <Engine.Resource/ResourceManager.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "Importer/AudioFileTypes.hpp"
#include "Importer/FlacImporter.hpp"
#include "Importer/VorbisImporter.hpp"
#include "Importer/WavImporter.hpp"

using namespace ember::engine;
using namespace ember;

ptr<Audio> Audio::_instance = nullptr;

ptr<Audio> Audio::get() noexcept {
    return _instance;
}

ptr<Audio> Audio::make() {
    if (!_instance) {
        _instance = new Audio();
    }
    return _instance;
}

void Audio::destroy() noexcept {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

Audio::Audio() noexcept {}

Audio::~Audio() noexcept {}

void Audio::setupImporter() {

    SCOPED_STOPWATCH

    //
    auto manager = ResourceManager::make();

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
