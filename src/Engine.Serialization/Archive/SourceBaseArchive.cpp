#include "SourceBaseArchive.hpp"

#include <format>

using namespace hg::engine::serialization;
using namespace hg;

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

fs::Url SourceBaseArchive::getArchiveUrl() const noexcept {
    return fs::Url {
        fs::Path {
            _STD format(
                "SourceArchive<<{:#016x}>>",
                reinterpret_cast<ptrdiff_t>(_source.get())
            )
        }
    };
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
    // TODO: Check why tell is not supported?!? return -1;
    return _pos;
}
