#include "pch.h"

/**/

import Engine.Logging;

/**/

namespace {
	// Note: Mutations log on commit, and the logger dereferences null until setup ran.
	// Note: A test engine sets up and destroys the logger itself, so such tests drop ours first.
	class LoggingListener :
		public testing::EmptyTestEventListener {
	public:
		void OnTestStart(const testing::TestInfo&) override {
			::hg::Logger::setup();
		}

		void OnTestEnd(const testing::TestInfo&) override {
			::hg::Logger::destroy();
		}
	};

	[[maybe_unused]] const auto registered = [] {
		testing::UnitTest::GetInstance()->listeners().Append(new LoggingListener {});
		return true;
	}();
}
