#include "LogSink.hpp"

#include <Engine.Diagnostics/Render.hpp>

#include "Logger.hpp"

using namespace hg::engine::diag;
using namespace hg;

void LogSink::emit(mref<Diagnostic> diagnostic_) {
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
