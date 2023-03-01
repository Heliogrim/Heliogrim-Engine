#include "SourceReadonlyArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

SourceReadonlyArchive::SourceReadonlyArchive(
    mref<smr<resource::Source>> source_,
    mref<streamoff> srcOff_,
    mref<streamsize> srcSize_
) :
    SourceBaseArchive(
        _STD move(source_),
        _STD move(srcOff_),
        _STD move(srcSize_)
    ) {
    assert(_source->isReadable());
}

SourceReadonlyArchive::~SourceReadonlyArchive() = default;

void SourceReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eIn) [[unlikely]]
    {
        return;
    }

    auto* const source = _source.get();
    streamsize readSize = 0ui64;

    const auto success = source->get(_srcOff + _pos, size_, value_, readSize);

    if (not success || readSize != size_) {
        setError();
    } else {
        _pos += readSize;
    }
}
