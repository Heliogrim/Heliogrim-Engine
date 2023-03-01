#include "StyleChainEntry.hpp"

using namespace hg::engine::reflow;
using namespace hg;

StyleChainEntry::StyleChainEntry() :
    _key(),
    _cnd(nullptr),
    _style() {}

StyleChainEntry::StyleChainEntry(cref<key_type> key_, cref<cnd_type> cnd_, cref<sptr<StyleSheet>> style_) :
    _key(key_),
    _cnd(cnd_),
    _style(style_) {}

StyleChainEntry::StyleChainEntry(cref<this_type> other_) :
    _key(other_._key),
    _cnd(other_._cnd),
    _style(other_._style) {}

StyleChainEntry::StyleChainEntry(mref<this_type> other_) noexcept :
    _key(_STD exchange(other_._key, {})),
    _cnd(_STD exchange(other_._cnd, nullptr)),
    _style(_STD move(other_._style)) {}

StyleChainEntry::~StyleChainEntry() = default;

ref<StyleChainEntry::this_type> StyleChainEntry::operator=(cref<this_type> other_) {

    if (this != _STD addressof(other_)) {
        _key = other_._key;
        _cnd = other_._cnd;
        _style = other_._style;
    }

    return *this;
}

ref<StyleChainEntry::this_type> StyleChainEntry::operator=(mref<this_type> other_) noexcept {

    if (this != _STD addressof(other_)) {
        _key = _STD exchange(other_._key, {});
        _cnd = _STD exchange(other_._cnd, nullptr);
        _style = _STD move(other_._style);
    }

    return *this;

}

cref<StyleChainEntry::key_type> StyleChainEntry::key() const noexcept {
    return _key;
}

cref<StyleChainEntry::cnd_type> StyleChainEntry::cnd() const noexcept {
    return _cnd;
}

cref<sptr<StyleSheet>> StyleChainEntry::style() const noexcept {
    return _style;
}
