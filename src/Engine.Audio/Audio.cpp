#include "Audio.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Resource/ResourceManager.hpp>

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

Audio::Audio(ref<Engine> engine_) noexcept :
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

	const auto manager { _engine->getResources() };

	//
	manager->importer().registerImporter(audio::AudioFileType::Flac, make_sptr<audio::FlacImporter>());
	manager->importer().registerImporter(audio::AudioFileType::Ogg, make_sptr<audio::VorbisImporter>());
	manager->importer().registerImporter(audio::AudioFileType::Wav, make_sptr<audio::WavImporter>());
}

void Audio::unregisterImporter() {

	const auto manager { _engine->getResources() };

	/**/
	manager->importer().unregisterImporter(audio::AudioFileType::Flac);
	manager->importer().unregisterImporter(audio::AudioFileType::Ogg);
	manager->importer().unregisterImporter(audio::AudioFileType::Wav);
}
