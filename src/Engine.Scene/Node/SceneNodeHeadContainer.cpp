#include "SceneNodeHeadContainer.hpp"

#include <algorithm>

using namespace hg::engine::scene;
using namespace hg;

SceneNodeHeadContainer::pointer_type SceneNodeHeadContainer::unsafe_base() const noexcept {
    return const_cast<pointer_type>(_base);
}

SceneNodeHeadContainer::pointer_type SceneNodeHeadContainer::unsafe_last() const noexcept {
    return _last;
}

constexpr cref<SceneNodeHeadContainer::allocator_type> SceneNodeHeadContainer::getAlloc() const noexcept {
    return _alloc;
}

constexpr ref<SceneNodeHeadContainer::allocator_type> SceneNodeHeadContainer::getAlloc() noexcept {
    return _alloc;
}

constexpr void SceneNodeHeadContainer::destroy(pointer_type from_, pointer_type to_) {
    _STD _Destroy_range(from_, to_, getAlloc());
}

constexpr void SceneNodeHeadContainer::tidy() noexcept {
    if (_last != _base) {
        if (_last > _base) {
            destroy(_base, _last);
        }
        // Inplace Container should never call dealloc on memory sequence getAlloc().deallocate(_base, static_cast<size_type>(_end - _base));

        // TODO: _base = nullptr;
        _last = _base;
    }
}

SceneNodeHeadContainer::SceneNodeHeadContainer() noexcept (_STD is_nothrow_constructible_v<SceneNodeHead>) :
    _alloc(),
    _base {},
    _last(_base) {}

SceneNodeHeadContainer::SceneNodeHeadContainer(mref<SceneNodeHeadContainer> other_) noexcept :
    _alloc(other_._alloc) {

    if (other_._base != other_._last) {
        _STD ranges::move(other_._base, other_._last, _base);
    }

    _last = &_base[other_.size()];
    other_._last = other_._base;
}

SceneNodeHeadContainer::~SceneNodeHeadContainer() noexcept {
    tidy();
}

ref<SceneNodeHeadContainer> SceneNodeHeadContainer::operator=(mref<SceneNodeHeadContainer> other_) noexcept {
    if (&other_ != this) {
        _STD ranges::move(other_._base, other_._last, _base);

        _last = &_base[other_.size()];
        other_._last = other_._base;
    }

    return *this;
}

bool SceneNodeHeadContainer::empty() const noexcept {
    return _base == _last;
}

SceneNodeHeadContainer::size_type SceneNodeHeadContainer::size() const noexcept {
    return static_cast<size_type>(_last - _base);
}

constexpr SceneNodeHeadContainer::size_type SceneNodeHeadContainer::max_size() const noexcept {
    #if FALSE
    return static_cast<size_type>(_end - _base);
    #endif
    return type_traits::max_nodes_per_layer;
}

cref<SceneNodeHeadContainer::value_type> SceneNodeHeadContainer::operator[](size_t pos_) const {
    #if _CONTAINER_DEBUG_LEVEL > 0
    _STL_VERIFY(pos_ < static_cast<size_type>(_last - _base), "container subscription of range");
    #endif

    return _base[pos_];
}

ref<SceneNodeHeadContainer::value_type> SceneNodeHeadContainer::operator[](size_t pos_) {
    #if _CONTAINER_DEBUG_LEVEL > 0
    _STL_VERIFY(pos_ < static_cast<size_type>(_last - _base), "container subscription of range");
    #endif

    return _base[pos_];
}

void SceneNodeHeadContainer::clear() {
    tidy();
}

void SceneNodeHeadContainer::swap(ptr<value_type> left_, ptr<value_type> right_) noexcept {
    // Make copy from left_
    value_type tmp { _STD forward<value_type>(*left_) };
    left_->value_type::~value_type();

    // Move right_ to left_
    new(left_) value_type { _STD forward<value_type>(*right_) };
    right_->value_type::~value_type();

    // Move copy to right_
    new(right_) value_type { _STD forward<value_type>(tmp) };
}

bool SceneNodeHeadContainer::push(mref<value_type> node_) {
    return emplace<value_type>(_STD move(node_));
}

void SceneNodeHeadContainer::push(ptr<value_type> first_, const ptr<value_type> end_) {
    for (; first_ != end_; ++first_) {
        emplace<value_type>(_STD move(*first_));
    }
}

bool SceneNodeHeadContainer::pull(ptr<value_type> pulled_) noexcept {
    if (empty()) {
        return false;
    }

    new(pulled_) value_type { _STD forward<value_type>(*--_last) };
    _last->value_type::~value_type();

    return true;
}

bool SceneNodeHeadContainer::pull(cref<value_id_type> nodeId_, ptr<value_type> pulled) {

    const auto last = end();
    const auto lv = _last - size_type { 1 };

    auto iter = begin();
    for (; iter != last; ++iter) {
        if (iter->nodeId() == nodeId_) {
            break;
        }
    }

    if (iter == last) {
        return false;
    }

    if (iter._ptr != lv) {
        swap(iter._ptr, lv);
    }

    return pull(_STD forward<ptr<value_type>>(pulled));
}

bool SceneNodeHeadContainer::remove(iterator pos_) noexcept {

    const iterator::pointer_type wherePtr = pos_._ptr;

    _STD _Move_unchecked(wherePtr + 1, _last, wherePtr);
    allocator_traits::destroy(getAlloc(), _STD _Unfancy(_last - 1));
    --_last;

    return true;
}

bool SceneNodeHeadContainer::remove(iterator from_, iterator to_) noexcept {

    const iterator::pointer_type fromPtr = from_._ptr;
    const iterator::pointer_type toPtr = to_._ptr;

    if (fromPtr == toPtr) {
        return false;
    }

    const iterator::pointer_type newLast = _STD _Move_unchecked(toPtr, _last, fromPtr);
    destroy(newLast, _last);
    _last = newLast;

    return true;
}

SceneNodeHeadContainer::const_iterator SceneNodeHeadContainer::begin() const {
    return const_iterator { _base };
}

SceneNodeHeadContainer::iterator SceneNodeHeadContainer::begin() {
    return iterator { _base };
}

SceneNodeHeadContainer::const_iterator SceneNodeHeadContainer::end() const {
    return const_iterator { _last };
}

SceneNodeHeadContainer::iterator SceneNodeHeadContainer::end() {
    return iterator { _last };
}

SceneNodeHeadContainer::const_reverse_iterator SceneNodeHeadContainer::rbegin() const {
    return const_reverse_iterator { _last - 1 };
}

SceneNodeHeadContainer::reverse_iterator SceneNodeHeadContainer::rbegin() {
    return reverse_iterator { _last - 1 };
}

SceneNodeHeadContainer::const_reverse_iterator SceneNodeHeadContainer::rend() const {
    return const_reverse_iterator { _base - 1 };
}

SceneNodeHeadContainer::reverse_iterator SceneNodeHeadContainer::rend() {
    return reverse_iterator { _base - 1 };
}
