#include "SourceWriteonlyArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

SourceWriteonlyArchive::SourceWriteonlyArchive(
    mref<smr<resource::Source>> source_,
    mref<streamoff> srcOff_,
    mref<streamsize> srcSize_
) :
    SourceBaseArchive(
        std::move(source_),
        std::move(srcOff_),
        std::move(srcSize_)
    ) {
    assert(_source->isWritable());
}

SourceWriteonlyArchive::~SourceWriteonlyArchive() = default;

void SourceWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eOut) [[unlikely]]
    {
        return;
    }

    auto* const source = _source.get();
    streamsize wroteSize = 0;

    const auto success = source->write(_pos, size_, value_, wroteSize);

    if (not success || wroteSize != size_) {
        setError();
    } else {
        _pos += wroteSize;
    }
}
