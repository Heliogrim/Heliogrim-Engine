#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"

namespace hg::engine::diag {
	class DiagnosticSink {
	public:
		virtual ~DiagnosticSink() = default;

		virtual void emit(mref<Diagnostic> diagnostic_) = 0;
	};

	/**/

	class CollectingSink final : public DiagnosticSink {
	private:
		Diagnostics _items;
		bool _hasErrors = false;

	public:
		void emit(mref<Diagnostic> diagnostic_) override;

	public:
		[[nodiscard]] bool hasErrors() const noexcept;

		[[nodiscard]] ref<const Diagnostics> items() const noexcept;

		/**
		 * Hands out everything collected so far and resets the sink.
		 */
		[[nodiscard]] Diagnostics take();
	};
}
