#include "Crc32.hpp"

#define CRCPP_USE_CPP11
#include <crcpp/CRC.h>

using namespace hg::engine::crypto;
using namespace hg;

static auto crc32Table = CRC::CRC_32().MakeTable();

/**/

ref<Crc32> Crc32::update(std::span<_::byte> data_) {
	// @see: https://d-bahr.github.io/CRCpp/
	if (not _intermediate) {
		_intermediate = CRC::Calculate(data_.data(), data_.size(), crc32Table);
	} else {
		_intermediate = CRC::Calculate(data_.data(), data_.size(), crc32Table, _intermediate);
	}
	return *this;
}

Crc32::result_type Crc32::finalize() && {
	return _intermediate;
}
