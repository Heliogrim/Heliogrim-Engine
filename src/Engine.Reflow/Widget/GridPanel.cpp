#include "GridPanel.hpp"

using namespace hg::engine::reflow;
using namespace hg;

GridPanel::GridPanel() :
    Panel(),
    _children() {}

GridPanel::~GridPanel() = default;

const ptr<const Children> GridPanel::children() const {
    return &_children;
}

void GridPanel::addChild(cref<sptr<Widget>> child_) {

    child_->setParent(shared_from_this());
    _children.push_back(child_);

    markAsPending();
}

void GridPanel::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

    if (idx_ == _children.size()) {
        addChild(child_);
        return;
    }

    if (idx_ > _children.size()) {
        return;
    }

    child_->setParent(shared_from_this());
    _children[idx_]->setParent(nullptr);
    _children[idx_] = child_;
}

void GridPanel::removeChild(cref<sptr<Widget>> child_) {

    s32 found = -1i32;
    for (u32 idx = 0; idx < _children.size(); ++idx) {
        if (_children[idx] == child_) {
            found = idx;
            break;
        }
    }

    /**/

    if (found < 0) {
        return;
    }

    const auto iter = _children.cbegin() + found;

    (*iter)->setParent(nullptr);
    _children.erase(_children.cbegin() + found);

    markAsPending();
}

void GridPanel::clearChildren() {

    for (const auto& child : _children) {
        child->setParent(nullptr);
    }

    _children.clear();

    markAsPending();
}
