module;
module Engine.Logging;

import Engine.Diagnostics;

import :Logger;
import :LogSink;

using namespace hg::engine::diag;
using namespace hg;

static void log_one(ref<const Diagnostic> diagnostic_);

void LogSink::emit(mref<Diagnostic> diagnostic_) {
	log_one(diagnostic_);
}

void hg::engine::diag::logDiagnostics(ref<const Diagnostics> diagnostics_) {
	for (const auto& diagnostic : diagnostics_) {
		log_one(diagnostic);
	}
}

static void log_one(ref<const Diagnostic> diagnostic_) {
	auto text = render(diagnostic_);
	if (text.ends_with('\n')) {
		text.pop_back();
	}
	const auto severity = diagnostic_.severity();

	if (severity == Severity::eError) {
		Logger::error(std::move(text));
	} else if (severity == Severity::eWarning) {
		Logger::warn(std::move(text));
	} else {
		Logger::info(std::move(text));
	}
}
