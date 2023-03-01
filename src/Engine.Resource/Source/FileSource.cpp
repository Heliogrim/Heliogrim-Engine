#include "FileSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace hg::engine::resource;
using namespace hg;

FileSource::FileSource(mref<fs::File> file_, const streamsize size_, const streamoff offset_) noexcept :
    _file(_STD move(file_)),
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

streamsize FileSource::size() const {
    return _STD filesystem::file_size(_file.path());
}

streamsize FileSource::estimatedSize() const noexcept {
    return _STD filesystem::file_size(_file.path());
}

bool FileSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {

    SCOPED_STOPWATCH

    auto strPath = _file.path().string();
    #pragma warning(push)
    #pragma warning(disable : 4996)
    FILE* fptr = _STD fopen(strPath.c_str(), "rb");
    #pragma warning(pop)

    streamsize cmpSize = _size;
    if (cmpSize <= 0) {

        _STD fseek(fptr, 0, SEEK_END);
        const streamsize fsize = _STD ftell(fptr);

        cmpSize = fsize - _offset;
    }

    if (cmpSize <= 0 || _offset > cmpSize) {
        _STD fclose(fptr);
        return false;
    }

    const streampos begin = _offset + offset_;
    const streamsize length = MIN(size_, cmpSize);

    _STD fseek(fptr, begin, SEEK_SET);
    actualSize_ = _STD fread(dst_, sizeof(_::byte), length, fptr);

    _STD fclose(fptr);

    return true;
}

hg::concurrent::future<Source::async_result_value> FileSource::get(streamoff offset_, streamsize size_) {
    throw NotImplementedException {};
}

bool FileSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {

    SCOPED_STOPWATCH

    const string strPath = _file;
    #pragma warning(push)
    #pragma warning(disable : 4996)
    auto* fptr = _STD fopen(strPath.c_str(), "a+b");
    #pragma warning(pop)

    streamsize cmpSize = _size;
    if (cmpSize <= 0) {

        _STD fseek(fptr, 0, SEEK_END);
        const streamsize fsize = _STD ftell(fptr);

        cmpSize = fsize - _offset;
    }

    if (cmpSize <= 0 || _offset > cmpSize) {
        _STD fclose(fptr);
        return false;
    }

    const streampos begin = _offset + offset_;
    // TODO: Check whether we need to check for max-length attributes, which could cause trimmed sequences

    _STD fseek(fptr, begin, SEEK_SET);
    actualSize_ = _STD fwrite(src_, sizeof(_::byte), size_, fptr);

    _STD fclose(fptr);
    return true;
}

hg::concurrent::future<Source::async_write_result> FileSource::write(
    streamoff offset_,
    streamsize size_,
    const ptr<void> src_
) {
    throw NotImplementedException {};
}
