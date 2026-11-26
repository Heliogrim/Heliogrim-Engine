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

	diag::Diagnostic toDiagnostic(ref<const io::CommitError> error_, mref<String> blob_) {
		const auto unconfirmed = error_.replaced && error_.kind == io::CommitError::Kind::eFlushFailed;
		return diag::Diagnostic::error(
			codes::sto("package-commit-failed"),
			std::format("package commit failed for `{}`", error_.path.empty() ? blob_ : error_.path)
		).note(
			unconfirmed
				? std::format("{}: the new file is in place, but its durability is unconfirmed", io::kindName(error_.kind))
				: String { io::kindName(error_.kind) }
		);
	}

	String describe(ref<const io::CommitError> error_) {
		if (error_.replaced && error_.kind == io::CommitError::Kind::eFlushFailed) {
			return std::format(
				"commit failed ({}) at `{}`: the new file is in place, but its durability is unconfirmed",
				io::kindName(error_.kind),
				error_.path
			);
		}
		return std::format("commit failed ({}) at `{}`", io::kindName(error_.kind), error_.path);
	}
}
