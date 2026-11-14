#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Severity.hpp"

namespace hg::engine::diag {
	class SourceLookup {
	public:
		virtual ~SourceLookup() = default;

		[[nodiscard]] virtual Optional<StringView> find(StringView file_) const = 0;
	};

	/**/

	[[nodiscard]] StringView severityName(Severity severity_);

	/**
	 * Renders a diagnostic as plain text; with `sources_` every source span gets a one-line snippet.
	 */
	[[nodiscard]] String render(ref<const Diagnostic> diagnostic_, ptr<const SourceLookup> sources_ = nullptr);
}
