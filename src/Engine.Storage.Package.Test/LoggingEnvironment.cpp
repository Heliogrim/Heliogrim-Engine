#include "pch.h"

/**/

import Engine.Logging;

/**/

namespace {
	// Note: Mutations log on commit; the logger dereferences null until setup ran.
	class LoggingEnvironment :
		public testing::Environment {
	public:
		void SetUp() override {
			::hg::Logger::setup();
		}

		void TearDown() override {
			::hg::Logger::destroy();
		}
	};

	[[maybe_unused]] const auto* const registered = testing::AddGlobalTestEnvironment(new LoggingEnvironment {});
}
