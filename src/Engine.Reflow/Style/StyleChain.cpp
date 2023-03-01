#include "StyleChain.hpp"

#include <algorithm>
#include <ranges>

#include "../Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

StyleChain::StyleChain() :
    _list() {}

StyleChain::~StyleChain() = default;

bool StyleChain::empty() const noexcept {
    return _list.empty();
}

u64 StyleChain::size() const noexcept {
    return _list.size();
}

void StyleChain::pushStyle(mref<StyleChainEntry> style_) {
    _list.push_back(_STD move(style_));
}

void StyleChain::pushStyle(mref<StyleChainEntry> style_, const s32 off_) {

    decltype(_list)::iterator where {};
    if (off_ >= 0) {
        where = _list.begin() + off_;
    } else {
        // Attention: Be aware that `off_` is negative at this point -> `(iter)+(-off)`
        where = _list.end() + off_;
    }

    _list.insert(where, _STD move(style_));
}

void StyleChain::popStyle(cref<StyleChainEntry::key_type> style_) {

    const auto where = _STD remove_if(
        _list.begin(),
        _list.end(),
        [style = style_](const auto& entry_) {
            return entry_.key() == style;
        }
    );

    _list.erase(where, _list.end());
}
