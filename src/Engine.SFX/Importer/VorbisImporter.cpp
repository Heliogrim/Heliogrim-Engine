#include "VorbisImporter.hpp"

#include <cstring>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>
#include <vorbis/vorbisfile.h>

#include "AudioFileTypes.hpp"

using namespace hg::engine::sfx;
using namespace hg;

bool VorbisImporter::canImport(cref<res::FileTypeId> typeId_, cref<::hg::fs::File> file_) const noexcept {
	return typeId_ == AudioFileType::Ogg;
}

VorbisImporter::descriptor_type VorbisImporter::descriptor() const noexcept {
	return {};
}

VorbisImporter::import_result_type VorbisImporter::import(
	cref<res::FileTypeId> typeId_,
	cref<::hg::fs::File> file_
) const {

	OggVorbis_File vf;
	if (ov_fopen(static_cast<std::string>(file_.path()).c_str(), &vf) < 0) {
		// Error opening
		throw std::runtime_error("Could not open file to parse vorbis data.");
	}

	auto& info = *vf.vi;
	const u64 samples = ov_pcm_total(&vf, -1);

	Buffer buffer {
		.size = samples * info.channels * sizeof(float),
		.align = 0uLL,
		.mem = malloc(samples * info.channels * sizeof(float))
	};

	bool eof = false;
	int cs = 0;
	char chunk[4096];
	u64 dstOffset = 0;

	while (!eof) {

		auto read = ov_read(&vf, chunk, sizeof(chunk), 0, 2, 1, &cs);

		if (read == 0) {
			eof = true;

		} else if (read < 0) {
			if (read == OV_EBADLINK) {
				throw std::runtime_error("Corrupt bitstream section while reading vorbis file.");
			}

			// .. any other error, might not be problematic
		} else {
			std::memcpy(
				&(static_cast<ptr<char>>(buffer.mem)[dstOffset]),
				chunk,
				read
			);

			dstOffset += read;
		}
	}

	import_type result {
		.channels = static_cast<u8>(info.channels),
		.format = AudioFormat::eF32,
		.samples = samples,
		.sampleRate = static_cast<u64>(info.rate),
		.buffer = buffer
	};

	ov_clear(&vf);

	//
	auto state { make_sptr<concurrent::future_state<import_type>>() };
	state->set(std::move(result));

	return import_result_type { state };
}
