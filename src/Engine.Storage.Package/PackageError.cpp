#include "PackageError.hpp"

#include "Diagnostic/Codes.hpp"

namespace hg::engine::storage::package {
	diag::Diagnostic toDiagnostic(ref<const PackageError> error_, mref<String> blob_) {

		auto at = diag::ByteOffset { ::hg::move(blob_), error_.offset, error_.length };

		switch (error_.kind) {
			case PackageError::Kind::eChecksum: {
				return diag::Diagnostic::error(
					codes::sto("package-checksum"),
					"package checksum does not match its content"
				).at(::hg::move(at));
			}
			case PackageError::Kind::eNewer: {
				return diag::Diagnostic::error(
					codes::sto("package-newer"),
					"package was written by a newer version"
				).at(::hg::move(at));
			}
			default: {
				return diag::Diagnostic::error(
					codes::sto("package-malformed"),
					"package structure is invalid"
				).at(::hg::move(at));
			}
		}
	}
}
