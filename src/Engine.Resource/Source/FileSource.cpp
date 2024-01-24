#include "FileSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

#if ENV_WIN
#include <Windows.h>
#endif

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace hg::engine::resource;
using namespace hg;

FileSource::FileSource(mref<fs::File> file_, const streamsize size_, const streamoff offset_) noexcept :
    _file(std::move(file_)),
    _fptr(nullptr),
    _size(size_),
    _offset(offset_) {}

FileSource::~FileSource() noexcept {
    if (isOpenHandle()) {
        close();
    }
}

FileSource::reference_type FileSource::operator=(mref<value_type> other_) noexcept {
    if (this != std::addressof(other_)) {
        _file = std::move(other_._file);
        _fptr = std::exchange(other_._fptr, nullptr);
        _size = other_._size;
        _offset = other_._offset;
    }

    return *this;
}

bool FileSource::open(int flags_) {
    #if ENV_WIN

    if (not(flags_ & GENERIC_WRITE) && not _file.exists()) {
        return false;
    }

    _fptr = ::CreateFileW(
        _file.path().c_str(),
        flags_,
        NULL,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (_fptr == INVALID_HANDLE_VALUE) {
        return false;
    }

    return true;

    #else
    #endif
}

bool FileSource::close() {

    #if ENV_WIN

    if (_fptr == nullptr || _fptr == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (::CloseHandle(_fptr) == NULL) {
        const auto reason = ::GetLastError();
        return false;
    }

    _fptr = nullptr;
    return true;

    #else
    #endif
}

bool FileSource::isOpenHandle() const noexcept {
    return _fptr != nullptr && _fptr != INVALID_HANDLE_VALUE;
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
    return std::filesystem::file_size(_file.path());
}

streamsize FileSource::estimatedSize() const noexcept {
    return std::filesystem::file_size(_file.path());
}

bool FileSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {

    SCOPED_STOPWATCH

    if (not isOpenHandle() && not open(GENERIC_READ)) {
        return false;
    }

    streamsize cmpSize = _size;
    if (cmpSize <= 0) {

        const streamsize fsize = ::GetFileSize(_fptr, nullptr);
        cmpSize = fsize - _offset;
    }

    if (cmpSize <= 0 || _offset > cmpSize) {
        return false;
    }

    const streampos begin = _offset + offset_;
    const streamsize length = MIN(size_, cmpSize);

    auto succeeded = ::SetFilePointerEx(_fptr, LARGE_INTEGER { .QuadPart = begin }, nullptr, FILE_BEGIN);

    unsigned long read {};
    succeeded = ::ReadFile(_fptr, dst_, sizeof(_::byte) * length, &read, nullptr) != NULL;

    actualSize_ = read;
    return succeeded;
}

hg::concurrent::future<Source::async_result_value> FileSource::get(streamoff offset_, streamsize size_) {
    throw NotImplementedException {};
}

bool FileSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {

    SCOPED_STOPWATCH

    if (not isOpenHandle() && not open(GENERIC_READ | GENERIC_WRITE)) {
        return false;
    }

    if (_size > 0 && offset_ + size_ > _size) {
        return false;
    }

    /**/

    const streampos begin = _offset + offset_;
    const streamsize fsize = ::GetFileSize(_fptr, nullptr);

    if (fsize < begin) {

        Vector<char> dummy = Vector<char>(begin - fsize);

        ::OVERLAPPED meta {
            NULL,
            NULL,
            {
                static_cast<DWORD>(fsize),
                static_cast<DWORD>(fsize >> 32)
            },
            nullptr
        };

        unsigned long written {};
        const auto succeeded = ::WriteFile(
            _fptr,
            dummy.data(),
            sizeof(_::byte) * dummy.size(),
            &written,
            &meta
        );

        if (written != dummy.size()) {
            __debugbreak();
        }
    }

    /**/

    ::OVERLAPPED meta {
        NULL,
        NULL,
        {
            static_cast<DWORD>(begin),
            static_cast<DWORD>(begin >> 32)
        },
        nullptr
    };

    unsigned long written {};
    const auto succeeded = ::WriteFile(
        _fptr,
        src_,
        sizeof(_::byte) * size_,
        &written,
        &meta
    );

    actualSize_ = written;
    return succeeded;
}

hg::concurrent::future<Source::async_write_result> FileSource::write(
    streamoff offset_,
    streamsize size_,
    const ptr<void> src_
) {
    throw NotImplementedException {};
}
