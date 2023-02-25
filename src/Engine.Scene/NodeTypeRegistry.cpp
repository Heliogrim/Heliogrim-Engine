#include "NodeTypeRegistry.hpp"

using namespace ember::engine::scene;
using namespace ember;

NodeTypeRegistryValue NodeTypeRegistry::_empty = { nullptr, nullptr, nullptr };

void NodeTypeRegistry::sanitize(cref<component_type_id> typeId_) {

    auto mapped { _mapped.find(typeId_) };
    if (mapped == _mapped.end()) {
        return;
    }

    /**
     *
     */
    auto& kv { *mapped };
    auto& value { kv.second };

    if (value.gfx || value.pfx || value.sfx) {
        return;
    }

    /**
     *
     */
    _mapped.erase(mapped);
}

cref<NodeTypeRegistryValue> NodeTypeRegistry::get(cref<component_type_id> typeId_) noexcept {
    auto mapped { _mapped.find(typeId_) };
    if (mapped != _mapped.end()) {
        return mapped->second;
    }

    return _empty;
}

void NodeTypeRegistry::setGfxModel(
    cref<component_type_id> typeId_,
    cref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.gfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.gfx = callback_;
}

void NodeTypeRegistry::setGfxModel(
    cref<component_type_id> typeId_,
    mref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.gfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.gfx = _STD move(callback_);
}

void NodeTypeRegistry::setPfxModel(
    cref<component_type_id> typeId_,
    cref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.pfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.pfx = callback_;
}

void NodeTypeRegistry::setPfxModel(
    cref<component_type_id> typeId_,
    mref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.pfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.pfx = _STD move(callback_);
}

void NodeTypeRegistry::setSfxModel(
    cref<component_type_id> typeId_,
    cref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.sfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.sfx = callback_;
}

void NodeTypeRegistry::setSfxModel(
    cref<component_type_id> typeId_,
    mref<NodeTypeRegistryValue::model_type> callback_
) {

    /**
     *
     */
    if (callback_ == nullptr) {

        const auto mapped { _mapped.find(typeId_) };
        if (mapped != _mapped.end()) {
            mapped->second.sfx = nullptr;
        }

        return sanitize(typeId_);
    }

    /**
     *
     */
    auto& mapped { _mapped[typeId_] };
    mapped.sfx = _STD move(callback_);
}
