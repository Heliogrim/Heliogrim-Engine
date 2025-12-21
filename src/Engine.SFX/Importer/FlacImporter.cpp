#include "FlacImporter.hpp"

#include <Engine.Common/Buffer.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>

#define DR_FLAC_IMPLEMENTATION
#include <dr_libs/dr_flac.h>

#include "AudioFileTypes.hpp"

using namespace hg::engine::sfx;
using namespace hg;

bool FlacImporter::canImport(cref<res::FileTypeId> typeId_, cref<::hg::fs::File> file_) const noexcept {
	return typeId_ == AudioFileType::Flac;
}

FlacImporter::descriptor_type FlacImporter::descriptor() const noexcept {
	return {};
}

FlacImporter::import_result_type FlacImporter::import(
	ref<const res::FileTypeId> typeId_,
	ref<const hg::fs::File> file_,
	mref<res::ImportDestination> destination_
) const {

	const ptr<drflac> flac = drflac_open_file(static_cast<std::string>(file_.path()).c_str(), nullptr);
	if (flac == nullptr) {
		// Error opening
		throw std::runtime_error("Could not open file to parse flac data.");
	}

	Buffer buffer {
		.size = flac->totalPCMFrameCount * flac->channels * sizeof(float),
		.align = 0uLL,
		.mem = malloc(flac->totalPCMFrameCount * flac->channels * sizeof(float))
	};

	u64 decodedSamples =
		drflac_read_pcm_frames_f32(flac, flac->totalPCMFrameCount, static_cast<ptr<float>>(buffer.mem));

	import_type result {
		.channels = static_cast<u8>(flac->channels),
		.format = AudioFormat::eF32,
		.samples = decodedSamples,
		.sampleRate = flac->sampleRate,
		.buffer = buffer
	};

	drflac_close(flac);

	//
	auto state { make_sptr<concurrent::future_state<import_type>>() };
	state->set(std::move(result));

	return import_result_type { state };
}