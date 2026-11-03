#include "pch.h"

#include <Engine.Common/Meta/Relocatable.hpp>

using namespace hg;

namespace Common {
	struct PlainRel {
		int a;
	};

	struct PolyRel {
		virtual ~PolyRel() = default;
	};

	struct PinnedRel {};
}

template <>
struct hg::relocatable<Common::PinnedRel> : std::false_type {};

namespace Common {
	TEST(Relocatable, DefaultTrue) {
		EXPECT_TRUE(relocatable_v<PlainRel>);
		EXPECT_TRUE(relocatable_v<PolyRel>);
	}

	TEST(Relocatable, OptOut) {
		EXPECT_FALSE(relocatable_v<PinnedRel>);
	}
}
