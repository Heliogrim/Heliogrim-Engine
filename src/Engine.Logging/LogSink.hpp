#pragma once

#include <Engine.Diagnostics/Diagnostic.hpp>
#include <Engine.Diagnostics/Sink.hpp>

namespace hg::engine::diag {
	/**
	 * Forwards each diagnostic to the Logger; error maps to error, warning to warn, note and help to info.
	 */
	class LogSink final :
		public DiagnosticSink {
	public:
		void emit(mref<Diagnostic> diagnostic_) override;
	};

	/**
	 * Logs each diagnostic the way LogSink::emit does.
	 */
	void logDiagnostics(ref<const Diagnostics> diagnostics_);
}
