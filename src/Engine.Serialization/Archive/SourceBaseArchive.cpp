#include "SourceBaseArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SourceBaseArchive::SourceBaseArchive(mref<smr<res::Source>> source_) :
    _source(_STD move(source_)),
    _pos(0) {}

SourceBaseArchive::~SourceBaseArchive() noexcept = default;

Url SourceBaseArchive::getArchiveUrl() const noexcept {
    const string path {
        _STD format(
            "SourceArchive<<{:#016x}>>",
            reinterpret_cast<ptrdiff_t>(_source.unwrap())
        )
    };
    return Url { ""sv, path };
}

string_view SourceBaseArchive::getArchiveName() const noexcept {
    return "SourceArchive"sv;
}

smr<engine::res::Source> SourceBaseArchive::getSource() const noexcept {
    return {};
}

void SourceBaseArchive::seek(const s64 pos_) {
    _pos = pos_;
}

s64 SourceBaseArchive::tell() const noexcept {
    return -1;
}
