#include "File.hpp"

using namespace ember::fs;
using namespace ember;

constexpr File::File() = default;

File::File(mref<path_type> path_) noexcept :
    _path(_STD move(path_)) {}

File::File(cref<path_type> path_) noexcept :
    _path(path_) {}

File::File(cref<string> url_) :
    _path(url_) { }

File::File(cref<string_view> url_) :
    _path(_STD string(url_)) {}

File::File(mref<File> other_) noexcept :
    _path(_STD move(other_._path)) {}

File::File(cref<File> other_) noexcept :
    _path(other_._path) {}

File::~File() noexcept = default;

ref<File> File::operator=(mref<File> other_) noexcept {
    if (_STD addressof(other_) != this) {
        _path = _STD move(other_._path);
    }
    return *this;
}

ref<File> File::operator=(cref<File> other_) noexcept {
    if (_STD addressof(other_) != this) {
        _path = other_._path;
    }
    return *this;
}

cref<File::path_type> File::path() const {
    return _path;
}

bool File::exists() const {
    return _STD filesystem::exists(_path);
}

bool File::isDirectory() const {
    return _STD filesystem::is_directory(_path);
}

streamsize File::size() const {
    return _STD filesystem::file_size(_path);
}

File File::parent() const {
    return File { _path.parent_path() };
}

Vector<File> File::files() const {
    // Could return without effect
    if (!isDirectory()) {
        throw std::exception("URL is not a directory. Can not fetch files.");
    }

    Vector<File> files = Vector<File>(0);
    const auto iter = _STD filesystem::directory_iterator { _path };

    for (const auto& entry : iter) {
        if (entry.is_regular_file()) {
            files.push_back(File { entry.path().string() });
        }
    }

    return files;
}

void File::setPath(cref<File::path_type> path_) {
    _path = path_;
}

void File::mkdir() const {
    if (exists()) {
        throw std::exception("Directory already exists");
    }

    _STD filesystem::create_directory(_path);
}

void File::mkdirs() const {
    _STD filesystem::create_directories(_path);
}

void File::createFile() const {
    if (exists() || isDirectory()) {
        throw std::exception("File already exists or is a directory.");
    }

    /* at the moment std::ofstream will create our file for us */
    std::ofstream stream { _path };
    stream.close();
}

void File::move(const File& dst_) {
    if (!dst_.exists() || !dst_.isDirectory()) {
        throw std::exception("Target is not a existing directory.");
    }

    const auto copyResult = _STD filesystem::copy_file(_path, dst_._path, _STD filesystem::copy_options {});

    if (not copyResult) {
        return;
    }

    /**/

    _STD filesystem::remove(_path);
}

File::operator std::string() const noexcept {
    return _path.string();
}

bool File::operator==(cref<File> other_) const noexcept {
    return _path == other_._path;
}

bool File::operator!=(cref<File> other_) const noexcept {
    return _path != other_._path;
}
