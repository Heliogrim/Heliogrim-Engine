#include "SourceBaseArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SourceBaseArchive::SourceBaseArchive(
    mref<smr<resource::Source>> source_,
    mref<streamoff> srcOff_,
    mref<streamsize> srcSize_
) :
    _source(_STD move(source_)),
    _srcOff(_STD move(srcOff_)),
    _srcSize(_STD move(srcSize_)),
    _pos(0) {}

SourceBaseArchive::~SourceBaseArchive() noexcept = default;

Url SourceBaseArchive::getArchiveUrl() const noexcept {
    const string path {
        _STD format(
            "SourceArchive<<{:#016x}>>",
            reinterpret_cast<ptrdiff_t>(_source.get())
        )
    };
    return Url { ""sv, path };
}

string_view SourceBaseArchive::getArchiveName() const noexcept {
    return "SourceArchive"sv;
}

smr<engine::resource::Source> SourceBaseArchive::getSource() const noexcept {
    return {};
}

void SourceBaseArchive::seek(const s64 pos_) {
    _pos = pos_;
}

s64 SourceBaseArchive::tell() const noexcept {
    return -1;
}
