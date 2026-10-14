#include "pch.h"

#include <array>
#include <Engine.Common/Guid.hpp>

using namespace hg;

namespace Common {
	TEST(Guid, hostToNetworkWritesBigEndian) {
		const Guid guid { 0x01020304u, 0x0506u, 0x0708u, 0x090A0B0C0D0E0F10uLL };

		u8 bytes[16] {};
		guid.hostToNetwork(bytes);

		const std::array<u8, 16> expected {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
		};
		EXPECT_EQ(std::to_array(bytes), expected);
	}

	TEST(Guid, networkToHostRoundTrip) {
		const Guid guid { 0xDEADBEEFu, 0x1234u, 0x5678u, 0x0123456789ABCDEFuLL };

		u8 raw[16] {};
		guid.hostToNetwork(raw);

		EXPECT_EQ(Guid::networkToHost(raw), guid);
	}
}
