#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Math/Transform.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>

#include "SceneNodeHeadContainer.hpp"
#include "SceneNodeVersion.hpp"

namespace ember::engine::scene {

    /**
     * Forward Declaration
     *
     * @author Julius
     * @date 15.08.2021
     */
    class SceneNodeHead;

    template <class PayloadType_>
    class ShadowSceneNode;

    template <class PayloadType_>
    class LoosySceneNode;

    template <class PayloadType_>
    class SpartialSceneNode;

    template <class PayloadType_>
    class NaturalSceneNode;

    template <class NodeStorageType_, class ElementStorageType_>
    class SceneNodeFactory;

    /**
     * A pull result type.
     *
     * @author Julius
     * @date 16.08.2021
     */
    struct pull_result_type {

        using result_type = bool;
        using value_type = ptr<SceneNodeHead>;

        result_type result;
        value_type value;

    public:
        ~pull_result_type() noexcept {
            tidy();
        }

    private:
        constexpr void tidy() noexcept {
            if (value) {
                delete value;
                value = nullptr;
            }
        }

    public:
        constexpr void reset() noexcept {
            result = false;
            tidy();
        }

    public:
        [[nodiscard]] operator bool() const noexcept {
            return result;
        }

        [[nodiscard]] operator bool() noexcept {
            return result;
        }

        [[nodiscard]] bool operator!() const noexcept {
            return !result;
        }

        [[nodiscard]] bool operator!() noexcept {
            return !result;
        }

    public:
        constexpr pull_result_type() noexcept :
            result(false),
            value(nullptr) { }

        constexpr pull_result_type(cref<value_type> value_) noexcept(_STD is_nothrow_copy_constructible_v<value_type>) :
            result(true),
            value(value_) {}

        constexpr pull_result_type(mref<value_type> value_) noexcept(_STD is_nothrow_move_constructible_v<value_type>) :
            result(true),
            value(_STD move(value_)) {}

        template <typename PTy_>
        [[nodiscard]] static pull_result_type make(
            mref<PTy_> value_) noexcept(_STD is_nothrow_move_constructible_v<PTy_>) {
            return pull_result_type {
                new PTy_(_STD forward<PTy_>(value_))
            };
        }

    public:
        pull_result_type(cref<pull_result_type>) = delete;

        pull_result_type(mref<pull_result_type> other_) noexcept :
            result(_STD exchange(other_.result, false)),
            value(_STD exchange(other_.value, nullptr)) { }

        ref<pull_result_type> operator=(cref<pull_result_type>) = delete;

        ref<pull_result_type> operator=(mref<pull_result_type> other_) noexcept {
            if (this != &other_) {
                result = _STD exchange(other_.result, result);
                value = _STD exchange(other_.value, value);
            }

            return *this;
        }
    };

    template <class PayloadType_>
    class SceneNode {
    public:
        friend class SceneNodeHead;

    public:
        using this_type = SceneNode<PayloadType_>;
        using smart_version_type = SceneNodeVersion;
        using container_type = SceneNodeHeadContainer;

        using shadow_type = ShadowSceneNode<PayloadType_>;
        using loosy_type = LoosySceneNode<PayloadType_>;
        using spartial_type = SpartialSceneNode<PayloadType_>;
        using natural_type = NaturalSceneNode<PayloadType_>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        SceneNode() noexcept :
            _version(),
            _children(),
            _elements(nullptr),
            _elementCount(0),
            _state(SceneNodeState::eShadow),
            _size(1),
            _transform(),
            _bounding() {}

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  other_ The other.
         */
        SceneNode(mref<this_type> other_) noexcept :
            _version(_STD move(other_._version)),
            _children(_STD move(other_._children)),
            _elements(_STD move(other_._elements)),
            _elementCount(_STD move(other_._elementCount)),
            _state(_STD exchange(other_._state, SceneNodeState::eShadow)),
            _size(_STD move(other_._size)),
            _transform(_STD move(other_._transform)),
            _bounding(_STD move(other_._bounding)) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        SceneNode(cref<this_type>) = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        ~SceneNode() noexcept = default;

        /**
         * Copy Assignment
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @returns A shallow copy of this.
         */
        ref<this_type> operator=(cref<this_type>) = delete;

        /**
         * Move Assignment
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<this_type> operator=(mref<this_type> other_) noexcept {

            if (_STD addressof(other_) != this) {
                _version = _STD move(other_._version);
                _children = _STD move(other_._children);
                _elements = _STD move(other_._elements);
                _elementCount = _STD move(other_._elementCount);
                _state = _STD exchange(other_._state, SceneNodeState::eShadow);
                _size = _STD exchange(other_._size, 0);
                _transform = _STD move(other_._transform);
                _bounding = _STD move(other_._bounding);
            }

            return *this;
        }

    protected:
        /**
         * Node Version
         */
        smart_version_type _version;

    protected:
        /**
         * Children
         */
        container_type _children;

    public:
        /**
         * Gets the children of the current node
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A const reference to the children container
         */
        [[nodiscard]] cref<decltype(_children)> children() const noexcept {
            return _children;
        }

        /**
         * Gets the children of the current node
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A reference to the children container
         */
        [[nodiscard]] ref<decltype(_children)> children() noexcept {
            return _children;
        }

    protected:
        PayloadType_* _elements;
        u64 _elementCount;

    public:
        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A const reference to the stored payload
         */
        [[nodiscard]] cref<decltype(_elements)> elements() const noexcept {
            return _elements;
        }

        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A reference to the stored payload
         */
        [[nodiscard]] ref<decltype(_elements)> elements() noexcept {
            return _elements;
        }

        [[nodiscard]] u64 exclusiveElementCount() const noexcept {
            return _elementCount;
        }

        [[nodiscard]] u64 inclusiveElementCount() const {
            throw NotImplementedException();
        }

    protected:
        /** The state */
        SceneNodeState _state;

    public:
        /**
         * Node state
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;SceneNodeState&gt;
         */
        [[nodiscard]] cref<SceneNodeState> nodeState() const noexcept {
            return _state;
        }

        /**
         * Query if this is shadow node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if shadow node, false if not.
         */
        [[nodiscard]] bool isShadowNode() const noexcept {
            return _state == SceneNodeState::eShadow;
        }

        [[nodiscard]] ptr<const shadow_type> asShadow() const noexcept;

        [[nodiscard]] ptr<shadow_type> asShadow() noexcept;

        /**
         * Query if this is loosy node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if loosy node, false if not.
         */
        [[nodiscard]] bool isLoosyNode() const noexcept {
            return _state == SceneNodeState::eLoosy;
        }

        [[nodiscard]] ptr<const loosy_type> asLoosy() const noexcept;

        [[nodiscard]] ptr<loosy_type> asLoosy() noexcept;

        /**
         * Query if this is spartial node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if spartial node, false if not.
         */
        [[nodiscard]] bool isSpartialNode() const noexcept {
            return _state == SceneNodeState::eSpartial;
        }

        [[nodiscard]] ptr<const spartial_type> asSpartial() const noexcept;

        [[nodiscard]] ptr<spartial_type> asSpartial() noexcept;

        /**
         * Query if this is natural node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if natural node, false if not.
         */
        [[nodiscard]] bool isNaturalNode() const noexcept {
            return _state == SceneNodeState::eNatural;
        }

        [[nodiscard]] ptr<const natural_type> asNatural() const noexcept;

        [[nodiscard]] ptr<natural_type> asNatural() noexcept;

    public:
        /**
         * Query if this current node is leaf in graph
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns True if leaf, false if not.
         */
        [[nodiscard]] bool isLeaf() const noexcept {
            return _children.empty();
        }

        /**
         * Gets the max depth from current node in graph
         *
         * @author Julius
         * @date 01.08.2021
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 depth() const noexcept {

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

    protected:
        // TODO: Recheck
        [[deprecated]] u64 _size;

    public:
        [[nodiscard]] u64 exclusiveSize() const noexcept {
            return _children.size();
        }

        /**
         * Traces the accumulated count of elements in graph from current node
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @returns The accumulated count of elements.
         */
        [[nodiscard]] u64 inclusiveSize() const noexcept {
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

        /**
         * Query if this contains the given id_ as descendant
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @param  id_ The SceneNodeId to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(cref<SceneNodeId> id_) const noexcept {
            advanced_proxy_macro(contains, id_)
        }

        /**
         * Query if this contains the given other_ as descendant
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  other_ The const_reference_type to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(cref<this_type> other_) const noexcept {
            advanced_proxy_macro(contains, other_)
        }

    public:
        template <class FactoryType_>
        bool push(const ptr<PayloadType_> element_, cref<math::Bounding> boundary_,
            const ptr<const FactoryType_> factory_) {
            advanced_proxy_macro(push, element_, boundary_, factory_)
        }

        [[nodiscard]] pull_result_type pull(cref<SceneNodeId> nodeId_) noexcept {
            fadvanced_proxy_macro(pull, pull_result_type, nodeId_)
        }

        bool pullChained(cref<this_type> parent_, ptr<SceneNodeHead> pos_) noexcept {
            advanced_proxy_macro(pullChained, parent_, _STD forward<ptr<SceneNodeHead>>(pos_))
        }

    protected:
        math::Transform _transform;
        math::Bounding _bounding;

    public:
        [[nodiscard, deprecated]] ref<math::Transform> transform() noexcept {
            return _transform;
        }

        [[nodiscard, deprecated]] ref<math::Bounding> bounding() noexcept {
            return _bounding;
        }

    public:
        [[nodiscard]] bool intersects(cref<this_type> node_) const noexcept {
            advanced_proxy_macro(intersects, node_)
        }

        [[nodiscard]] bool intersects(cref<math::Bounding> boundary_) const noexcept {
            advanced_proxy_macro(intersects, boundary_)
        }

        [[nodiscard]] bool intersectsFully(cref<this_type> node_) const noexcept {
            advanced_proxy_macro(intersectsFully, node_)
        }

        [[nodiscard]] bool intersectsFully(cref<math::Bounding> boundary_) const noexcept {
            advanced_proxy_macro(intersectsFully, boundary_)
        }

        #undef proxy_macro
        #undef advanced_proxy_macro

    public:
        /**
         * Equality operator
         *
         * @author Julius
         * @date 12.07.2021
         *
         * @param  left_ The first instance to compare.
         * @param  right_ The second instance to compare.
         *
         * @returns True if the parameters are considered equivalent.
         */
        friend bool operator==(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
            return &left_ == &right_;
        }

        /**
         * Inequality operator
         *
         * @author Julius
         * @date 12.07.2021
         *
         * @param  left_ The first instance to compare.
         * @param  right_ The second instance to compare.
         *
         * @returns True if the parameters are not considered equivalent.
         */
        friend bool operator!=(cref<SceneNode> left_, cref<SceneNode> right_) noexcept {
            return &left_ != &right_;
        }
    };
}
