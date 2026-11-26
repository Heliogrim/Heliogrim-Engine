#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Diagnostics;
import Engine.IO.Dispatcher;
#endif
#pragma once

namespace hg::engine::storage::package {
	/**
	 * Inner result of opening a package, converted to a `STO` diagnostic at the storage boundary.
	 */
	struct PackageError {
		enum class Kind : u8 {
			eMalformed,
			eChecksum,
			eNewer
		};

		Kind kind;
		u64 offset;
		u64 length;
	};

	[[nodiscard]] diag::Diagnostic toDiagnostic(ref<const PackageError> error_, mref<String> blob_);

	[[nodiscard]] diag::Diagnostic toDiagnostic(ref<const io::CommitError> error_, mref<String> blob_);

	/**
	 * Why a commit failed, for a log line.
	 */
	[[nodiscard]] String describe(ref<const io::CommitError> error_);

	/**
	 * Why a mutation failed, for a log line; names the commit failure when there is one.
	 */
	[[nodiscard]] String describeMutationFailure(auto& error_) {
		if (error_.template is<io::CommitError>()) {
			return describe(error_.template as<io::CommitError>());
		}
		return String { "mutation failed" };
	}
}
