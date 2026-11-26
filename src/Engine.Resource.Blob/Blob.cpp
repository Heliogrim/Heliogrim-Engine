#include "Blob.hpp"

using namespace hg::engine::resource;
using namespace hg;

namespace io = hg::engine::io;

Result<Rc<Blob>, io::CommitError> Blob::beginReplace(streamsize) {
	return Result<Rc<Blob>, io::CommitError>::unexpected(io::CommitError { .kind = io::CommitError::Kind::eUnsupportedVolume });
}

Result<void, io::CommitError> Blob::finishReplace(mref<Rc<Blob>>) {
	return Result<void, io::CommitError>::unexpected(io::CommitError { .kind = io::CommitError::Kind::eUnsupportedVolume });
}
