#include "FileSource.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::res;
using namespace ember;

FileSource::FileSource(cref<File> file_, const u64 size_, const u64 offset_) noexcept :
    _file(file_),
    _size(size_),
    _offset(offset_) {}

FileSource::reference_type FileSource::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _file = _STD move(other_._file);
        _size = other_._size;
        _offset = other_._offset;
    }

    return *this;
}

bool FileSource::isAsync() const noexcept {
    return false;
}

bool FileSource::isReady() const noexcept {
    // TODO: Replace which logically correct statement (if source should be written to new file, file might not exist)
    return _file.exists();
}

bool FileSource::isReadable() const noexcept {
    return true;
}

bool FileSource::isWritable() const noexcept {
    return _size <= 0ui64;
}

bool FileSource::get(const u64 offset_, const u64 size_, ptr<void> dst_, ref<u64> actualSize_) {

    SCOPED_STOPWATCH

    _STD ifstream stream {};
    stream.open(_file, _STD ifstream::in | _STD ifstream::binary);
    stream.seekg(0, _STD ios::end);

    u64 cmpSize = _size;
    if (cmpSize <= 0) {
        stream.seekg(0, _STD ios::end);
        const u64 fsize = stream.tellg();
        stream.seekg(0, _STD ios::beg);

        cmpSize = fsize - _offset;
    }

    if (_offset > cmpSize) {
        return false;
    }

    const u64 begin = _offset * offset_;
    const u64 length = MIN(size_, cmpSize);

    stream.seekg(begin);

    stream.read(static_cast<ptr<char>>(dst_), length);
    actualSize_ = length;

    stream.close();

    return true;
}

concurrent::future<Source::async_result_value> FileSource::get(u64 offset_, u64 size_) {
    throw NotImplementedException {};
}
