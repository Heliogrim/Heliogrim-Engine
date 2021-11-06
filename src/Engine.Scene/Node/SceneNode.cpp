#include "SceneNode.hpp"

#include "ShadowSceneNode.hpp"
#include "LoosySceneNode.hpp"
#include "SpartialSceneNode.hpp"
#include "NaturalSceneNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::scene;
using namespace ember;

SceneNode::SceneNode() noexcept :
    _version(),
    _children(),
    _payload(),
    _state(SceneNodeState::eShadow),
    _size(1ui64),
    _transformation(math::ZeroTransformation {}),
    _bounding() {}

SceneNode::SceneNode(mref<value_type> other_) noexcept :
    _version(_STD exchange(other_._version, invalid_node_version {})),
    _children(_STD move(other_._children)),
    _payload(_STD move(other_._payload)),
    _state(_STD exchange(other_._state, SceneNodeState::eShadow)),
    _size(other_._size),
    _transformation(math::ZeroTransformation {}),
    _bounding() {}

SceneNode::~SceneNode() noexcept = default;

SceneNode::reference_type SceneNode::operator=(mref<value_type> other_) noexcept {
    if (this != &other_) {
        _children = _STD move(other_._children);
        _version = _STD exchange(other_._version, _version);
        _size = _STD exchange(other_._size, _size);
    }

    return *this;
}

cref<SceneNode::container_type> SceneNode::children() const noexcept {
    return _children;
}

ref<SceneNode::container_type> SceneNode::children() noexcept {
    return _children;
}

cref<engine::proxy::ProxiedScenePayload> SceneNode::payload() const noexcept {
    return _payload;
}

ref<engine::proxy::ProxiedScenePayload> SceneNode::payload() noexcept {
    return _payload;
}

cref<SceneNodeState> SceneNode::nodeState() const noexcept {
    return _state;
}

bool SceneNode::isShadowNode() const noexcept {
    return _state == SceneNodeState::eShadow;
}

ptr<const ShadowSceneNode> SceneNode::asShadow() const noexcept {
    return ShadowSceneNode::castFrom(this);
}

ptr<ShadowSceneNode> SceneNode::asShadow() noexcept {
    return ShadowSceneNode::castFrom(this);
}

bool SceneNode::isLoosyNode() const noexcept {
    return _state == SceneNodeState::eLoosy;
}

ptr<const LoosySceneNode> SceneNode::asLoosy() const noexcept {
    return LoosySceneNode::castFrom(this);
}

ptr<LoosySceneNode> SceneNode::asLoosy() noexcept {
    return LoosySceneNode::castFrom(this);
}

bool SceneNode::isSpartialNode() const noexcept {
    return _state == SceneNodeState::eSpartial;
}

ptr<const SpartialSceneNode> SceneNode::asSpartial() const noexcept {
    return SpartialSceneNode::castFrom(this);
}

ptr<SpartialSceneNode> SceneNode::asSpartial() noexcept {
    return SpartialSceneNode::castFrom(this);
}

bool SceneNode::isNaturalNode() const noexcept {
    return _state == SceneNodeState::eNatural;
}

ptr<const NaturalSceneNode> SceneNode::asNatural() const noexcept {
    return NaturalSceneNode::castFrom(this);
}

ptr<NaturalSceneNode> SceneNode::asNatural() noexcept {
    return NaturalSceneNode::castFrom(this);
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
        const auto res = entry.get()->depth();
        tmp = MAX(tmp, res);
    }

    return tmp + 1ui64;
}

u64 SceneNode::size() const noexcept {

    /*
    if (isLeaf()) {
        return 1ui64;
    }

    u64 tmp = 1ui64;
    for (const auto& entry : _children) {
        tmp += entry.get()->size();
    }

    return tmp;
    */
    return _size;
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

bool SceneNode::contains(cref<SceneNodeId> id_) const noexcept {
    advanced_proxy_macro(contains, id_)
}

bool SceneNode::contains(const_reference_type other_) const noexcept {
    advanced_proxy_macro(contains, other_)
}

bool SceneNode::push(_Inout_ mref<SceneNodeCreateData> data_, _In_ const ptr<const SceneNodeFactory> factory_) {
    advanced_proxy_macro(push, _STD forward<SceneNodeCreateData>(data_), factory_)
}

bool SceneNode::push(_In_ const ptr<SceneNodeCreateData> data_, _In_ const ptr<const SceneNodeFactory> factory_) {
    advanced_proxy_macro(push, data_, factory_)
}

pull_result_type SceneNode::pull(cref<SceneNodeId> nodeId_) noexcept {
    fadvanced_proxy_macro(pull, pull_result_type, nodeId_)
}

bool SceneNode::pullChained(cref<SceneNode> parent_, IN OUT ptr<SceneNodeHead> pos_) noexcept {
    advanced_proxy_macro(pullChained, parent_, _STD forward<ptr<SceneNodeHead>>(pos_))
}

bool SceneNode::intersects(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersects, node_)
}

bool SceneNode::intersects(cref<SceneNodeCreateData> data_) const noexcept {
    advanced_proxy_macro(intersects, data_)
}

bool SceneNode::intersectsFully(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersectsFully, node_)
}

bool SceneNode::intersectsFully(cref<SceneNodeCreateData> data_) const noexcept {
    advanced_proxy_macro(intersectsFully, data_)
}

bool SceneNode::intersectedFully(const_reference_type node_) const noexcept {
    advanced_proxy_macro(intersectedFully, node_)
}

#undef proxy_macro
#undef advanced_proxy_macro

bool engine::scene::operator==(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
    return &left_ == &right_;
}

bool engine::scene::operator!=(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
    return &left_ != &right_;
}
