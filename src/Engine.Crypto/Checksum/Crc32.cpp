module;

#include <zlib/zlib.h>

/**/

module Engine.Crypto;

import :Checksum.Crc32;

using namespace hg::engine::crypto;
using namespace hg;

ref<Crc32> Crc32::update(std::span<const _::byte> data_) {
	// Note: zlib's crc32_z chains from the finished value (start 0), CRC-32 IEEE as the package format fixes.
	// Note: An empty span may carry nullptr, which crc32_z reads as "return the initial value 0".
	if (data_.empty()) {
		return *this;
	}
	_intermediate = static_cast<result_type>(
		::crc32_z(_intermediate, reinterpret_cast<const Bytef*>(data_.data()), static_cast<z_size_t>(data_.size()))
	);
	return *this;
}

Crc32::result_type Crc32::finalize() && {
	return _intermediate;
}
