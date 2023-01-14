#include "SourceReadWriteArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SourceReadWriteArchive::SourceReadWriteArchive(mref<smr<res::Source>> source_) :
    SourceBaseArchive(_STD move(source_)) {
    assert(_source->isReadable() && _source->isWritable());
}

SourceReadWriteArchive::~SourceReadWriteArchive() = default;

void SourceReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError()) [[unlikely]]
    {
        return;
    }

    if (mode_ == ArchiveStreamMode::eIn) {

        auto* const source = _source.unwrap();
        streamsize wroteSize = 0;

        const auto success = source->write(_pos, size_, value_, wroteSize);

        if (not success || wroteSize != size_) {
            setError();
        } else {
            _pos += wroteSize;
        }

    } else {

        auto* const source = _source.unwrap();
        streamsize readSize = 0ui64;

        const auto success = source->get(_pos, size_, value_, readSize);

        if (not success || readSize != size_) {
            setError();
        } else {
            _pos += readSize;
        }
    }
}
