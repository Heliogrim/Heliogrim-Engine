
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Diagnostics;
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
}
