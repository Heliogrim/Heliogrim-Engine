#include "SourceWriteonlyArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SourceWriteonlyArchive::SourceWriteonlyArchive(mref<smr<res::Source>> source_) :
    SourceBaseArchive(_STD move(source_)) {
    assert(_source->isWritable());
}

SourceWriteonlyArchive::~SourceWriteonlyArchive() = default;

void SourceWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eOut) [[unlikely]]
    {
        return;
    }

    auto* const source = _source.unwrap();
    streamsize wroteSize = 0;

    const auto success = source->write(_pos, size_, value_, wroteSize);

    if (not success || wroteSize != size_) {
        setError();
    } else {
        _pos += wroteSize;
    }
}
