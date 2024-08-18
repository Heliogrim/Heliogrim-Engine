#include "File.hpp"

#include <stdexcept>

using namespace hg::fs;
using namespace hg;

File::File(mref<path_type> path_) noexcept :
	_path(std::move(path_)) {}

File::File(cref<path_type> path_) noexcept :
	_path(path_) {}

File::File(cref<string> url_) :
	_path(url_) {}

File::File(cref<string_view> url_) :
	_path(std::string(url_)) {}

File::File(mref<File> other_) noexcept :
	_path(std::move(other_._path)) {}

File::File(cref<File> other_) noexcept :
	_path(other_._path) {}

File::~File() noexcept = default;

ref<File> File::operator=(mref<File> other_) noexcept {
	if (std::addressof(other_) != this) {
		_path = std::move(other_._path);
	}
	return *this;
}

ref<File> File::operator=(cref<File> other_) noexcept {
	if (std::addressof(other_) != this) {
		_path = other_._path;
	}
	return *this;
}

cref<File::path_type> File::path() const {
	return _path;
}

bool File::exists() const {
	return std::filesystem::exists(_path);
}

bool File::isDirectory() const {
	return std::filesystem::is_directory(_path);
}

streamsize File::size() const {
	return std::filesystem::file_size(_path);
}

File File::parent() const {
	return File { _path.parentPath() };
}

Vector<File> File::files() const {
	// Could return without effect
	if (!isDirectory()) {
		throw std::runtime_error("URL is not a directory. Can not fetch files.");
	}

	Vector<File> files = Vector<File>(0);
	const auto iter = std::filesystem::directory_iterator { _path };

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
		throw std::runtime_error("Directory already exists");
	}

	std::filesystem::create_directory(_path);
}

void File::mkdirs() const {
	std::filesystem::create_directories(_path);
}

void File::createFile() const {
	if (exists() || isDirectory()) {
		throw std::runtime_error("File already exists or is a directory.");
	}

	/* at the moment std::ofstream will create our file for us */
	std::ofstream stream { static_cast<std::filesystem::path>(_path) };
	stream.close();
}

void File::move(const File& dst_) {
	if (!dst_.exists() || !dst_.isDirectory()) {
		throw std::runtime_error("Target is not a existing directory.");
	}

	const auto copyResult = std::filesystem::copy_file(_path, dst_._path, std::filesystem::copy_options {});

	if (not copyResult) {
		return;
	}

	/**/

	std::filesystem::remove(_path);
}

File::operator std::string() const noexcept {
	return static_cast<std::string>(_path);
}

bool File::operator==(cref<File> other_) const noexcept {
	return _path == other_._path;
}

bool File::operator!=(cref<File> other_) const noexcept {
	return _path != other_._path;
}
