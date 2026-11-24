
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif
#pragma once


namespace hg::engine::serialization {
	struct ReadError {
		enum class Kind : u8 {
			eMalformed,
			eTruncated,
			eDepth,
			eBudget,
			eTypeMismatch,
			eInvalidValue,
			eDuplicateKey,
			eMissingField,
			eDuplicateIdentity,
			eUnknownTypeRoot,
			eRevisionNewerRoot,
			eUnknownType,
			eUnknownField,
			eRevisionNewer,
			eUnresolvedReference,
			eMigrationFailed
		};

		Kind kind;
		s64 offset;
		s64 length;
		String detail;
	};
}
