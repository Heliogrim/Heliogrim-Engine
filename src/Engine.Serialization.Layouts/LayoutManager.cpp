#include "LayoutManager.hpp"

#include "Engine.Serialization/Layout/DataLayout.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"

using namespace hg::engine::serialization;
using namespace hg;

uptr<LayoutManager> LayoutManager::_instance = nullptr;

ref<LayoutManager> LayoutManager::get() noexcept {
    return *_instance;
}

ref<LayoutManager> LayoutManager::make() {

    if (not _instance) {
        _instance = uptr<LayoutManager>(new LayoutManager());
        _instance->setup();
    }

    return *_instance;
}

void LayoutManager::destroy() {
    _instance.reset();
}

LayoutManager::LayoutManager() :
    _optimizer(),
    _nullLayout(nullptr),
    _typeMapping(),
    _classMapping(),
    _symbolMapping() {}

LayoutManager::~LayoutManager() {
    tidy();
}

void LayoutManager::setup() {

    sptr<DataLayoutBase> ndl = make_sptr<DataLayout<double>>();
    ndl->describe();

    storeLayout("double"sv, ndl);
    storeLayout("double"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<float>>();
    ndl->describe();

    storeLayout("float"sv, ndl);
    storeLayout("float"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<s8>>();
    ndl->describe();

    storeLayout("s8"sv, ndl);
    storeLayout("s8"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<s16>>();
    ndl->describe();

    storeLayout("s16"sv, ndl);
    storeLayout("s16"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<s32>>();
    ndl->describe();

    storeLayout("s32"sv, ndl);
    storeLayout("s32"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<s64>>();
    ndl->describe();

    storeLayout("s64"sv, ndl);
    storeLayout("s64"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<u8>>();
    ndl->describe();

    storeLayout("u8"sv, ndl);
    storeLayout("u8"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<u16>>();
    ndl->describe();

    storeLayout("u16"sv, ndl);
    storeLayout("u16"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<u32>>();
    ndl->describe();

    storeLayout("u32"sv, ndl);
    storeLayout("u32"_typeId, ndl);

    /**/
    ndl = make_sptr<DataLayout<u64>>();
    ndl->describe();

    storeLayout("u64"sv, ndl);
    storeLayout("u64"_typeId, ndl);
}

void LayoutManager::tidy() {

    for (auto& pair : _classMapping) {
        pair.second->tidy();
    }

    for (auto& pair : _typeMapping) {
        pair.second->tidy();
    }

    for (auto& pair : _symbolMapping) {
        pair.second->tidy();
    }

    _classMapping.clear();
    _typeMapping.clear();
    _symbolMapping.clear();
}

void LayoutManager::optimize(cref<sptr<DataLayoutBase>> layout_) const {
    // TODO:
}

bool LayoutManager::storeLayout(cref<string> symbol_, cref<sptr<DataLayoutBase>> layout_) {

    const auto key { AssocKey<string>::from(symbol_) };
    if (_symbolMapping.contains(key)) {
        return false;
    }

    const auto result = _symbolMapping.insert_or_assign(key, layout_);
    return result.second;
}

bool LayoutManager::storeLayout(string_view symbol_, cref<sptr<DataLayoutBase>> layout_) {
    return LayoutManager::storeLayout(string { symbol_ }, layout_);
}

bool LayoutManager::storeLayout(const ptr<const MetaClass> class_, cref<sptr<DataLayoutBase>> layout_) {

    if (_classMapping.contains(class_)) {
        return false;
    }

    const auto result = _classMapping.insert_or_assign(class_, layout_);
    return result.second;
}

bool LayoutManager::storeLayout(cref<type_id> typeId_, cref<sptr<DataLayoutBase>> layout_) {

    if (_typeMapping.contains(typeId_)) {
        return false;
    }

    const auto result = _typeMapping.insert_or_assign(typeId_, layout_);
    return result.second;
}

cref<sptr<DataLayoutBase>> LayoutManager::getLayout(cref<string> symbol_) const noexcept {
    const auto it { _symbolMapping.find(AssocKey<string>::from(symbol_)) };
    if (it != _symbolMapping.end()) [[likely]]
    {
        return it->second;
    }

    return _nullLayout;
}

cref<sptr<DataLayoutBase>> LayoutManager::getLayout(string_view symbol_) const noexcept {
    return LayoutManager::getLayout(string { symbol_ });
}

cref<sptr<DataLayoutBase>> LayoutManager::getLayout(const ptr<const MetaClass> class_) const noexcept {
    const auto it { _classMapping.find(class_) };
    if (it != _classMapping.end()) [[likely]]
    {
        return it->second;
    }

    return _nullLayout;
}

cref<sptr<DataLayoutBase>> LayoutManager::getLayout(cref<type_id> typeId_) const noexcept {
    const auto it { _typeMapping.find(typeId_) };
    if (it != _typeMapping.end()) [[likely]]
    {
        return it->second;
    }

    return _nullLayout;
}
