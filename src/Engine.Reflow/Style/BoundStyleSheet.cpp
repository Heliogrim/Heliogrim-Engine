#include "BoundStyleSheet.hpp"

#include <Engine.Common/Make.hpp>

#include "StyleResolver.hpp"

using namespace ember::engine::reflow;
using namespace ember;

static const AssocKey<string> unique { AssocKey<string>::from("unique") };

BoundStyleSheet::BoundStyleSheet() :
    StyleSheet(),
    _styleChain() {}

BoundStyleSheet::BoundStyleSheet(cref<StyleSheet> local_) :
    StyleSheet(local_),
    _styleChain() {}

BoundStyleSheet::BoundStyleSheet(mref<StyleSheet> local_) :
    StyleSheet(_STD move(local_)),
    _styleChain() {}

BoundStyleSheet::BoundStyleSheet(cref<sptr<StyleSheet>> local_) :
    StyleSheet(*local_),
    _styleChain() {}

BoundStyleSheet::BoundStyleSheet(mref<this_type> other_) :
    StyleSheet(static_cast<mref<StyleSheet>>(other_)),
    _styleChain(_STD move(other_._styleChain)) {}

BoundStyleSheet::~BoundStyleSheet() = default;

sptr<BoundStyleSheet> BoundStyleSheet::make() {
    auto self { sptr<BoundStyleSheet>(new BoundStyleSheet()) };
    self->sanitize();
    return self;
}

sptr<BoundStyleSheet> BoundStyleSheet::make(cref<StyleSheet> local_) {
    auto self { sptr<BoundStyleSheet>(new BoundStyleSheet(local_)) };
    self->sanitize();
    return self;
}

sptr<BoundStyleSheet> BoundStyleSheet::make(mref<StyleSheet> local_) {
    auto self { sptr<BoundStyleSheet>(new BoundStyleSheet(_STD move(local_))) };
    self->sanitize();
    return self;
}

sptr<BoundStyleSheet> BoundStyleSheet::make(cref<sptr<StyleSheet>> local_) {
    auto self { sptr<BoundStyleSheet>(new BoundStyleSheet(*local_)) };
    self->sanitize();
    return self;
}

sptr<BoundStyleSheet> BoundStyleSheet::make(mref<this_type> local_) {
    auto self { sptr<BoundStyleSheet>(new BoundStyleSheet(_STD move(local_))) };
    self->sanitizeMove();
    return self;
}

void BoundStyleSheet::sanitize() {

    StyleChainEntry entry {
        unique,
        nullptr,
        shared_from_this()
    };

    _styleChain.pushStyle(_STD move(entry));
}

void BoundStyleSheet::sanitizeMove() {
    _styleChain.popStyle(unique);
    sanitize();
}

void BoundStyleSheet::pushStyle(mref<StyleChainEntry> style_) {
    _styleChain.pushStyle(_STD move(style_), -1);
}

void BoundStyleSheet::popStyle(cref<StyleChainEntry::key_type> style_) {
    _styleChain.popStyle(style_);
}

StyleSheet BoundStyleSheet::compute(cref<sptr<Widget>> widget_, ref<StyleKeyStack> styleStack_) {
    const StyleResolver<> resolver {};
    return resolver(_styleChain, widget_, styleStack_, static_cast<cref<StyleSheet>>(*this));
}
