#include "SceneNode.hpp"

#include "SceneNodeFactory.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNode::SceneNode(mref<value_type> other_) noexcept :
    _nodeId(_STD move(other_._nodeId)),
    _state(_STD exchange(other_._state, SceneNodeState::eShadow)),
    _children(_STD move(other_._children)),
    _element(_STD move(other_._element)),
    _transform(_STD move(other_._transform)),
    _bounding(_STD move(other_._bounding)) {}

SceneNode::reference_type SceneNode::operator=(mref<value_type> other_) noexcept {
    if (this != &other_) {
        _nodeId = _STD move(other_._nodeId);
        _state = _STD exchange(other_._state, SceneNodeState::eShadow);
        _children = _STD move(other_._children);
        _element = _STD move(other_._element);
        _transform = _STD move(other_._transform);
        _bounding = _STD move(other_._bounding);
    }

    return *this;
}

SceneNode::SceneNode(cref<node_id_type> nodeId_, cref<SceneNodeState> state_, mref<container_type> children_,
    cref<wptr<SceneElement>> element_, cref<math::Transformation> transform_, cref<math::Bounding> bounding_) :
    _nodeId(nodeId_),
    _state(state_),
    _children(_STD forward<container_type>(children_)),
    _element(element_),
    _transform(transform_),
    _bounding(bounding_) {}

SceneNode::~SceneNode() noexcept = default;

cref<SceneNode::node_id_type> SceneNode::nodeId() const noexcept {
    return _nodeId;
}

cref<SceneNodeState> SceneNode::nodeState() const noexcept {
    return _state;
}

bool SceneNode::isShadowNode() const noexcept {
    return _state == SceneNodeState::eShadow;
}

ptr<const SceneShadowNode> SceneNode::asShadow() const noexcept {
    return SceneShadowNode::castFrom(this);
}

ptr<SceneShadowNode> SceneNode::asShadow() noexcept {
    return SceneShadowNode::castFrom(this);
}

bool SceneNode::isLoosyNode() const noexcept {
    return _state == SceneNodeState::eLoosy;
}

ptr<const SceneLoosyTreeNode> SceneNode::asLoosy() const noexcept {
    return SceneLoosyTreeNode::castFrom(this);
}

ptr<SceneLoosyTreeNode> SceneNode::asLoosy() noexcept {
    return SceneLoosyTreeNode::castFrom(this);
}

bool SceneNode::isSpartialNode() const noexcept {
    return _state == SceneNodeState::eSpartial;
}

ptr<const SceneSpartialTreeNode> SceneNode::asSpartial() const noexcept {
    return SceneSpartialTreeNode::castFrom(this);
}

ptr<SceneSpartialTreeNode> SceneNode::asSpartial() noexcept {
    return SceneSpartialTreeNode::castFrom(this);
}

bool SceneNode::isNaturalNode() const noexcept {
    return _state == SceneNodeState::eNatural;
}

ptr<const SceneNaturalTreeNode> SceneNode::asNatural() const noexcept {
    return SceneNaturalTreeNode::castFrom(this);
}

ptr<SceneNaturalTreeNode> SceneNode::asNatural() noexcept {
    return SceneNaturalTreeNode::castFrom(this);
}

cref<SceneNode::container_type> SceneNode::children() const noexcept {
    return _children;
}

ref<SceneNode::container_type> SceneNode::children() noexcept {
    return _children;
}

bool SceneNode::isLeaf() const noexcept {
    return _children.empty();
}

u64 SceneNode::depth() const noexcept {

    if (isLeaf()) {
        return 0ui64;
    }

    u64 tmp = 0ui64;
    for (const auto& entry : _children) {
        const auto res = entry.depth();
        tmp = MAX(tmp, res);
    }

    return tmp + 1ui64;
}

u64 SceneNode::size() const noexcept {

    if (isLeaf()) {
        return 1ui64;
    }

    u64 tmp = 1ui64;
    for (const auto& entry : _children) {
        tmp += entry.size();
    }

    return tmp;
}

#define advanced_proxy_macro(FNC_, ...) \
    switch (_state) { \
        case SceneNodeState::eShadow: return asShadow()->FNC_(__VA_ARGS__); \
        case SceneNodeState::eLoosy: return asLoosy()->FNC_(__VA_ARGS__); \
        case SceneNodeState::eSpartial: return asSpartial()->FNC_(__VA_ARGS__); \
        case SceneNodeState::eNatural: return asNatural()->FNC_(__VA_ARGS__); \
        default: return false; \
    }

#define fadvanced_proxy_macro(FNC_, RTY_, ...) \
    switch (_state) { \
        case SceneNodeState::eShadow: return _STD forward<RTY_>(asShadow()->FNC_(__VA_ARGS__)); \
        case SceneNodeState::eLoosy: return _STD forward<RTY_>(asLoosy()->FNC_(__VA_ARGS__)); \
        case SceneNodeState::eSpartial: return _STD forward<RTY_>(asSpartial()->FNC_(__VA_ARGS__)); \
        case SceneNodeState::eNatural: return _STD forward<RTY_>(asNatural()->FNC_(__VA_ARGS__)); \
        default: return _STD forward<RTY_>(asShadow()->FNC_(__VA_ARGS__)); \
    }

bool SceneNode::contains(const_reference_type node_) const noexcept {
    advanced_proxy_macro(contains, node_)
}

bool SceneNode::contains(cref<node_id_type> id_) const noexcept {
    advanced_proxy_macro(contains, id_)
}

bool SceneNode::push(mref<SceneNode> node_) {
    advanced_proxy_macro(push, _STD forward<value_type>(node_))
}

pull_result_type SceneNode::pull(
    cref<node_id_type> id_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v<
    value_type>) {
    fadvanced_proxy_macro(pull, pull_result_type, id_)
}

pull_result_type SceneNode::pull(
    const_reference_type node_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v
    <value_type>) {
    fadvanced_proxy_macro(pull, pull_result_type, node_)
}

bool SceneNode::pullChained(const_reference_type parent_,
    pointer_type pos_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v<
    value_type>) {
    advanced_proxy_macro(pullChained, parent_, _STD forward<pointer_type>(pos_))
}

bool SceneNode::intersects(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersects, node_)
}

bool SceneNode::intersectsFully(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersectsFully, node_)
}

bool SceneNode::intersectedFully(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersectedFully, node_)
}

#undef proxy_macro
#undef advanced_proxy_macro

cref<wptr<SceneElement>> SceneNode::element() const noexcept {
    return _element;
}

cref<wptr<SceneElement>> SceneNode::element() noexcept {
    return _element;
}

bool ember::engine::scene::operator==(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
    return left_._nodeId == right_._nodeId;
}

bool ember::engine::scene::operator!=(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
    return left_._nodeId != right_._nodeId;
}

cref<math::Transformation> SceneNode::transform() const noexcept {
    return _transform;
}

ref<math::Transformation> SceneNode::transform() noexcept {
    return _transform;
}

cref<math::Bounding> SceneNode::bounding() const noexcept {
    return _bounding;
}

ref<math::Bounding> SceneNode::bounding() noexcept {
    return _bounding;
}

bool SceneShadowNode::contains(const_reference_type node_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    for (const auto& entry : _children) {
        if (entry == node_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (entry.intersects(node_))
                return entry.contains(node_);
        } else {
            if (entry.intersects(node_) &&
                entry.contains(node_)) {
                return true;
            }
        }
    }

    return false;
}

template <typename ContainerType_, typename NodeIdType_>
FORCE_INLINE constexpr bool contains_by_id(cref<ContainerType_> container_, NodeIdType_ id_) noexcept {
    if (container_.empty()) {
        return false;
    }

    // First check for direct member
    for (const auto& entry : container_) {
        if (entry.nodeId() == id_) {
            return true;
        }
    }

    // Check for indirect member
    for (const auto& entry : container_) {
        if (entry.contains(id_)) {
            return true;
        }
    }

    return false;
}

bool SceneShadowNode::contains(cref<node_id_type> id_) const noexcept {
    return contains_by_id(_children, id_);
}

bool SceneShadowNode::push(mref<SceneNode> node_) {
    #ifdef _DEBUG
    if (_children.empty() && _children.unsafe_base() == nullptr) {
        throw _STD runtime_error("Node Container does not contain a valid or pre-allocated memory sequence.");
    }
    #endif

    // Container is full -> need to insert into descendant
    if (_children.size() >= type_trait::max_nodes_per_layer) {

        // Preference every other node than shadow node
        for (auto& entry : _children) {
            if (entry.isShadowNode())
                continue;

            if constexpr (type_trait::distinct_intersect) {
                if (entry.intersectsFully(node_)) {
                    return entry.push(_STD move(node_));
                }
            } else {
                // TODO: consider memory pressure
                if (entry.intersectsFully(node_) && entry.push(_STD move(node_))) {
                    return true;
                }
            }
        }

        // Fallback to shadow node
        u64 lowestSize = ~0;
        container_type::iterator cur = _children.end();

        const container_type::iterator end = _children.end();
        container_type::iterator iter = _children.begin();

        for (; iter != end; ++iter) {
            if (!iter->isShadowNode())
                continue;

            const auto testSize = iter->size();
            if (testSize <= 1ui64) {
                cur = iter;
                break;
            }

            if (testSize < lowestSize) {
                cur = iter;
                lowestSize = testSize;
            }
        }

        // TODO: consider memory pressure
        return cur->push(_STD move(node_));
    }

    // Container has some space left -> insert directly
    return _children.push(_STD move(node_));
}

pull_result_type SceneShadowNode::pull(
    cref<node_id_type> id_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v<
    value_type>) {

    if (isLeaf()) {
        return {};
    }

    auto iter = _children.begin();
    const auto last = _children.end();

    // Check for direct child node
    for (; iter != last; ++iter) {
        if (iter->nodeId() == id_)
            break;
    }

    // Check for indirect childs in sub-graph
    if (iter == last) {
        pull_result_type result {};
        for (iter = _children.begin(); iter != last; ++iter) {
            if (result = iter->pull(id_)) {
                return result;
            }
        }
    }

    // Guard child removal, if reference is not contained in this or sub-graph
    if (iter == last) {
        return {};
    }

    // TODO:
    ASSERT(iter->isShadowNode())

    //
    pull_result_type result = pull_result_type::make<value_type>(_STD move(*iter));

    if (!result.value->pullChained(*this, iter.pointer())) {
        #ifdef _DEBUG
        // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
        ASSERT(_children.remove(iter))
        #else
        _children.remove(iter)
        #endif
    }

    return result;
}

pull_result_type SceneShadowNode::pull(
    const_reference_type node_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v
    <value_type>) {

    if (isLeaf()) {
        return {};
    }

    auto iter = _children.begin();
    const auto last = _children.end();

    // Check for direct child node
    for (; iter != last; ++iter) {
        if (*iter == node_)
            break;
    }

    // Check for indirect childs in sub-graph
    if (iter == last) {
        pull_result_type result {};
        for (iter = _children.begin(); iter != last; ++iter) {

            if (result = iter->pull(node_)) {
                return result;
            }
        }
    }

    // Guard child removal, if reference is not contained in this or sub-graph
    if (iter == last) {
        return {};
    }

    // TODO:
    ASSERT(iter->isShadowNode())

    //
    pull_result_type result = pull_result_type::make<value_type>(_STD move(*iter));

    if (!result.value->pullChained(*this, iter.pointer())) {
        #ifdef _DEBUG
        // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
        ASSERT(_children.remove(iter))
        #else
        _children.remove(iter)
        #endif
    }

    return result;
}

bool SceneShadowNode::pullChained(const_reference_type parent_,
    pointer_type pos_) noexcept(_STD is_nothrow_move_assignable_v<value_type> && _STD is_nothrow_destructible_v<
    value_type>) {
    if (isLeaf()) {
        return false;
    }

    // In-Place replace with [biggest or leaf] shadow node might be best
    container_type::iterator target = _children.end();
    u64 maxSize = 0;

    auto iter = _children.begin();
    const auto last = _children.end();

    // Check for shadow nodes => Replacing Shadow Node with Shadow Node
    for (; iter != last; ++iter) {
        if (!iter->isShadowNode())
            continue;

        if (iter->isLeaf()) {
            target = iter;
            break;
        }

        const auto testSize = iter->size();
        if (testSize >= maxSize) {
            target = iter;
            maxSize = testSize;
        }
    }

    // When no other shadow node is available, check for other node
    if (target == last) {
        throw NotImplementedException();
    }

    // TODO:
    ASSERT(target->isShadowNode())

    // Replace in-place at parent iterator position
    /**
     * A
     * | \
     * |  \
     * B   C
     *    / \       /\ {Pushing Reference Up -> Forward}
     *   D   E
     */
    using value_type = container_type::value_type;

    pos_->value_type::~value_type();
    new(pos_) value_type { _STD move(*target) };

    // Check whether container element was replace or needs to be removed
    reference_type targetNode { *pos_ };

    if (!targetNode.pullChained(*this, target.pointer())) {
        #ifdef _DEBUG
        // Attention: This will invalidate the target iterator cause we modify the underlying container memory sequence
        ASSERT(_children.remove(target))
        #else
        _children.remove(target);
        #endif
    }

    // Push container to in-place target
    /**
     * A
     * | \
     * |  \
     * B   E[C]
     *    / \       \/ {Pushing Container Down -> Replace}
     *   D   -[E]
     */
    targetNode.children() = _STD move(_children);
    return true;
}

bool SceneShadowNode::intersects(const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}

bool SceneShadowNode::intersectsFully(const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}

bool SceneShadowNode::intersectedFully(const_reference_type node_) const noexcept {
    // Any node_ will intersect with shadow node
    return true;
}

bool SceneNaturalTreeNode::contains(const_reference_type node_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    // TODO: Replace
    for (const auto& entry : _children) {
        if (entry == node_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (entry.intersects(node_))
                return entry.contains(node_);
        } else {
            if (entry.intersects(node_) &&
                entry.contains(node_)) {
                return true;
            }
        }
    }

    return false;
}

bool SceneNaturalTreeNode::contains(cref<node_id_type> id_) const noexcept {
    return contains_by_id(_children, id_);
}

bool SceneNaturalTreeNode::push(mref<SceneNode> node_) {
    throw NotImplementedException();
}

pull_result_type SceneNaturalTreeNode::pull(
    cref<node_id_type> id_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

pull_result_type SceneNaturalTreeNode::pull(
    const_reference_type node_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

bool SceneNaturalTreeNode::pullChained(const_reference_type parent_, pointer_type pos_) {
    throw NotImplementedException();
}

bool SceneNaturalTreeNode::intersects(const_reference_type node_) const noexcept {
    return true;
}

bool SceneNaturalTreeNode::intersectsFully(const_reference_type node_) const noexcept {
    return true;
}

bool SceneNaturalTreeNode::intersectedFully(const_reference_type node_) const noexcept {
    return true;
}

bool SceneSpartialTreeNode::contains(const_reference_type node_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    // TODO: Replace
    for (const auto& entry : _children) {
        if (entry == node_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (entry.intersects(node_))
                return entry.contains(node_);
        } else {
            if (entry.intersects(node_) &&
                entry.contains(node_)) {
                return true;
            }
        }
    }

    return false;
}

bool SceneSpartialTreeNode::contains(cref<node_id_type> id_) const noexcept {
    return contains_by_id(_children, id_);
}

bool SceneSpartialTreeNode::push(mref<SceneNode> node_) {
    throw NotImplementedException();
}

pull_result_type SceneSpartialTreeNode::pull(
    cref<node_id_type> id_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

pull_result_type SceneSpartialTreeNode::pull(
    const_reference_type node_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

bool SceneSpartialTreeNode::pullChained(const_reference_type parent_, pointer_type pos_) {
    throw NotImplementedException();
}

bool SceneSpartialTreeNode::intersects(const_reference_type node_) const noexcept {
    return true;
}

bool SceneSpartialTreeNode::intersectsFully(const_reference_type node_) const noexcept {
    return true;
}

bool SceneSpartialTreeNode::intersectedFully(const_reference_type node_) const noexcept {
    return true;
}

bool SceneLoosyTreeNode::contains(const_reference_type node_) const noexcept {
    if (isLeaf()) {
        return false;
    }

    // TODO: Replace
    for (const auto& entry : _children) {
        if (entry == node_) {
            return true;
        }

        if constexpr (type_trait::distinct_intersect) {
            if (entry.intersects(node_))
                return entry.contains(node_);
        } else {
            if (entry.intersects(node_) &&
                entry.contains(node_)) {
                return true;
            }
        }
    }

    return false;
}

bool SceneLoosyTreeNode::contains(cref<node_id_type> id_) const noexcept {
    return contains_by_id(_children, id_);
}

bool SceneLoosyTreeNode::push(mref<SceneNode> node_) {
    throw NotImplementedException();
}

pull_result_type SceneLoosyTreeNode::pull(
    cref<node_id_type> id_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

pull_result_type SceneLoosyTreeNode::pull(
    const_reference_type node_) noexcept(_STD is_nothrow_move_assignable_v<value_type> &&
    _STD is_nothrow_destructible_v<value_type>) {
    throw NotImplementedException();
}

bool SceneLoosyTreeNode::pullChained(const_reference_type parent_, pointer_type pos_) {
    throw NotImplementedException();
}

bool SceneLoosyTreeNode::intersects(const_reference_type node_) const noexcept {
    return true;
}

bool SceneLoosyTreeNode::intersectsFully(const_reference_type node_) const noexcept {
    return true;
}

bool SceneLoosyTreeNode::intersectedFully(const_reference_type node_) const noexcept {
    return true;
}
