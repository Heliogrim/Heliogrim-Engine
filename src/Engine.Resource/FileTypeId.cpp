#include "FileTypeId.hpp"

using namespace ember::engine::res;
using namespace ember;

FileTypeId::FileTypeId(cref<type_id> typeId_, cref<string> ext_, const bool register_) :
    typeId(typeId_),
    ext(ext_) {

    if (register_) {
        FileTypeRegister::make().registerType(*this);
    }
}

bool FileTypeId::valid() const noexcept {
    return typeId.data != 0;
}

ptr<FileTypeRegister> FileTypeRegister::_instance = nullptr;

FileTypeRegister::FileTypeRegister() noexcept = default;

FileTypeRegister::~FileTypeRegister() noexcept = default;

ref<FileTypeRegister> FileTypeRegister::make() {

    if (!_instance) {
        _instance = new FileTypeRegister();
    }

    return *_instance;
}

ref<FileTypeRegister> FileTypeRegister::get() {
    return *_instance;
}

const ptr<FileTypeRegister> FileTypeRegister::get(std::nothrow_t) noexcept {
    return _instance;
}

void FileTypeRegister::destroy() {
    delete _instance;
    _instance = nullptr;
}

FileTypeId FileTypeRegister::getByTypeId(cref<type_id> id_) const noexcept {

    const auto found = _STD ranges::find_if(
        _list.begin(),
        _list.end(),
        [&id_](const auto& entry_) {
            return entry_.typeId == id_;
        }
    );

    if (found != _list.end()) {
        return *found;
    }

    return {};
}

FileTypeId FileTypeRegister::getByExt(string ext_) const noexcept {
    return getByExt(string_view { ext_ });
}

FileTypeId FileTypeRegister::getByExt(string_view ext_) const noexcept {

    const auto found = _STD ranges::find_if(
        _list.begin(),
        _list.end(),
        [&ext_](const auto& entry_) {
            return entry_.ext == ext_;
        }
    );

    if (found != _list.end()) {
        return *found;
    }

    return {};
}

void FileTypeRegister::registerType(cref<FileTypeId> type_) {
    // TODO: check for overlapping definition
    _list.push_back(type_);
}
