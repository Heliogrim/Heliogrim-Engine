#pragma once

#include <Engine.Common/__macro.hpp>

#include "Code.hpp"
#include "Fix.hpp"
#include "Location.hpp"
#include "Severity.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::diag {
	class DiagnosticBuilder;

	struct Label {
		Location at;
		String message;
	};

	class Diagnostic {
	private:
		friend class DiagnosticBuilder;

	public:
		struct Data {
			ptr<const DiagnosticCodeInfo> code;
			Severity severity;
			String message;
			Location primary;
			/* shortcut: Vector instead of InlineVector<Label, 2>, the latter has no copy or indexing support, upgrade when it gains them */
			Vector<Label> labels;
			Vector<String> notes;
			Vector<Fix> fixes;
		};

	public:
		Diagnostic() noexcept = default;

		Diagnostic(mref<Diagnostic> other_) noexcept = default;

		Diagnostic(ref<const Diagnostic> other_);

		~Diagnostic() noexcept = default;

	public:
		ref<Diagnostic> operator=(mref<Diagnostic> other_) noexcept = default;

		ref<Diagnostic> operator=(ref<const Diagnostic> other_);

	private:
		UniquePtr<Data> _data;

	public:
		void addNote(String note_);

		void setMessage(String message_);

		void setPrimary(Location primary_);

	public:
		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] ptr<const DiagnosticCodeInfo> code() const noexcept;

		[[nodiscard]] Severity severity() const noexcept;

		[[nodiscard]] ref<const String> message() const noexcept;

		[[nodiscard]] ref<const Location> primary() const noexcept;

		[[nodiscard]] ref<const Vector<Label>> labels() const noexcept;

		[[nodiscard]] ref<const Vector<String>> notes() const noexcept;

		[[nodiscard]] ref<const Vector<Fix>> fixes() const noexcept;

	public:
		[[nodiscard]] [[gnu::cold]] static DiagnosticBuilder error(ref<const DiagnosticCodeInfo> code_, String message_);

		[[nodiscard]] [[gnu::cold]] static DiagnosticBuilder warning(ref<const DiagnosticCodeInfo> code_, String message_);

		[[nodiscard]] [[gnu::cold]] static DiagnosticBuilder note(ref<const DiagnosticCodeInfo> code_, String message_);

		[[nodiscard]] [[gnu::cold]] static DiagnosticBuilder help(ref<const DiagnosticCodeInfo> code_, String message_);
	};

	using Diagnostics = Vector<Diagnostic>;

	/**/

	class DiagnosticBuilder {
	public:
		[[gnu::cold]] DiagnosticBuilder(Severity severity_, ref<const DiagnosticCodeInfo> code_, String message_);

	private:
		Diagnostic _diagnostic;

	public:
		[[gnu::cold]] DiagnosticBuilder&& at(Location location_) &&;

		[[gnu::cold]] DiagnosticBuilder&& label(Location location_, String message_) &&;

		[[gnu::cold]] DiagnosticBuilder&& note(String note_) &&;

		[[gnu::cold]] DiagnosticBuilder&& fix(Fix fix_) &&;

	public:
		[[gnu::cold]] operator Diagnostic() &&;

		[[gnu::cold]] operator Unexpected<Diagnostic>() &&;

		/* A single user conversion is allowed in copy-init, so `return builder;` needs a direct Result target. */
		template <typename Type_>
		[[gnu::cold]] operator Result<Type_, Diagnostic>() && {
			return Result<Type_, Diagnostic> { Unexpected<Diagnostic> { std::move(_diagnostic) } };
		}
	};
}
