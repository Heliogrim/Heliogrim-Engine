#include "SourceReadWriteArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

SourceReadWriteArchive::SourceReadWriteArchive(
    mref<smr<resource::Source>> source_,
    mref<streamoff> srcOff_,
    mref<streamsize> srcSize_
) :
    SourceBaseArchive(
        std::move(source_),
        std::move(srcOff_),
        std::move(srcSize_)
    ) {
    assert(_source->isReadable() && _source->isWritable());
}

SourceReadWriteArchive::~SourceReadWriteArchive() = default;

void SourceReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError()) [[unlikely]]
    {
        return;
    }

    if (mode_ == ArchiveStreamMode::eIn) {

        auto* const source = _source.get();
        streamsize wroteSize = 0;

        const auto success = source->write(_srcOff + _pos, size_, value_, wroteSize);

        if (not success || wroteSize != size_) {
            setError();
        } else {
            _pos += wroteSize;
        }

    } else {

        auto* const source = _source.get();
        streamsize readSize = 0uLL;

        const auto success = source->get(_srcOff + _pos, size_, value_, readSize);

        if (not success || readSize != size_) {
            setError();
        } else {
            _pos += readSize;
        }
    }
}
