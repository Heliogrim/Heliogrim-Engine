#include "pch.h"

/**/

import Engine.Common;
import Engine.Diagnostics;
import Engine.Logging;

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiag {
	namespace {
		inline constexpr DiagnosticCodeInfo testTable[] = {
			{ { { "TST" }, 1 }, "first-kind", "first" }
		};
	}

	/* Smoke test: Logger exposes no capture hook, so this only checks that every severity path runs. */
	TEST(LogSink, EmitsEachSeverity) {
		Logger::setup();

		LogSink sink {};
		const auto& code = findCode(testTable, "first-kind");
		sink.emit(Diagnostic::error(code, "e"));
		sink.emit(Diagnostic::warning(code, "w"));
		sink.emit(Diagnostic::note(code, "n"));
		sink.emit(Diagnostic::help(code, "h"));

		Logger::destroy();
	}
}
