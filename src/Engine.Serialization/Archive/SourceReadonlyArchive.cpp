#include "SourceReadonlyArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SourceReadonlyArchive::SourceReadonlyArchive(mref<smr<res::Source>> source_) :
    SourceBaseArchive(_STD move(source_)) {
    assert(_source->isReadable());
}

SourceReadonlyArchive::~SourceReadonlyArchive() = default;

void SourceReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eIn) [[unlikely]]
    {
        return;
    }

    auto* const source = _source.unwrap();
    streamsize readSize = 0ui64;

    const auto success = source->get(_pos, size_, value_, readSize);

    if (not success || readSize != size_) {
        setError();
    } else {
        _pos += readSize;
    }
}
