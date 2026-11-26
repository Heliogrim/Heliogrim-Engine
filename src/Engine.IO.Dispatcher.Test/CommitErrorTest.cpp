#include "pch.h"

/**/

import Engine.IO.Dispatcher;
import std;

using namespace hg;
using namespace hg::engine::io;

namespace IoModule {
	TEST(CommitError, KindOfOsError) {
		EXPECT_EQ(commitKindOf(112u), CommitError::Kind::eDiskFull);
		EXPECT_EQ(commitKindOf(39u), CommitError::Kind::eDiskFull);
		EXPECT_EQ(commitKindOf(32u), CommitError::Kind::eLocked);
		EXPECT_EQ(commitKindOf(33u), CommitError::Kind::eLocked);
		EXPECT_EQ(commitKindOf(5u), CommitError::Kind::eAccessDenied);
		EXPECT_EQ(commitKindOf(19u), CommitError::Kind::eReadOnly);
		EXPECT_EQ(commitKindOf(50u), CommitError::Kind::eUnsupportedVolume);
		EXPECT_EQ(commitKindOf(23u), CommitError::Kind::eDeviceError);
		EXPECT_EQ(commitKindOf(1117u), CommitError::Kind::eDeviceError);
		EXPECT_EQ(commitKindOf(12345u), CommitError::Kind::eUnknown);
	}

	TEST(CommitError, Scope) {
		using Kind = CommitError::Kind;
		for (const auto kind : { Kind::eDiskFull, Kind::eDeviceError, Kind::eFlushFailed, Kind::eUnsupportedVolume }) {
			EXPECT_EQ(CommitError { .kind = kind }.scope(), CommitError::Scope::eVolume);
		}
		for (const auto kind : { Kind::eLocked, Kind::eAccessDenied, Kind::eReadOnly, Kind::eDamagedSource, Kind::eUnknown }) {
			EXPECT_EQ(CommitError { .kind = kind }.scope(), CommitError::Scope::eFile);
		}
	}
}
