#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Math/Transformation.hpp>
#include <Engine.Proxy/ProxiedScenePayload.hpp>

#include "SceneNodeCreateData.hpp"
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

    struct pull_result_type;

    class ShadowSceneNode;
    class LoosySceneNode;
    class SpartialSceneNode;
    class NaturalSceneNode;

    class SceneNodeFactory;

    class SceneNode {
    public:
        friend class SceneNodeHead;

    public:
        using value_type = SceneNode;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;
        using pointer_type = ptr<value_type>;

        using smart_version_type = SceneNodeVersion;

        using container_type = SceneNodeHeadContainer;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        SceneNode() noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  other_ The other.
         */
        SceneNode(mref<value_type> other_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        SceneNode(const_reference_type) = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        ~SceneNode() noexcept;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept;

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
         * @returns A cref&lt;container_type&gt;
         */
        [[nodiscard]] cref<container_type> children() const noexcept;

        /**
         * Gets the children of the current node
         *
         * @author Julius
         * @date 15.08.2021
         *
         * @returns A ref&lt;container_type&gt;
         */
        [[nodiscard]] ref<container_type> children() noexcept;

    protected:
        /**
         * The stored payload_
         */
        proxy::ProxiedScenePayload _payload;

    public:
        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;proxy::ProxiedScenePayload&gt;
         */
        [[nodiscard]] cref<proxy::ProxiedScenePayload> payload() const noexcept;

        /**
         * Gets the payload
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A ref&lt;proxy::ProxiedScenePayload&gt;
         */
        [[nodiscard]] ref<proxy::ProxiedScenePayload> payload() noexcept;

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
        [[nodiscard]] cref<SceneNodeState> nodeState() const noexcept;

        /**
         * Query if this is shadow node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if shadow node, false if not.
         */
        [[nodiscard]] bool isShadowNode() const noexcept;

        [[nodiscard]] ptr<const ShadowSceneNode> asShadow() const noexcept;

        [[nodiscard]] ptr<ShadowSceneNode> asShadow() noexcept;

        /**
         * Query if this is loosy node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if loosy node, false if not.
         */
        [[nodiscard]] bool isLoosyNode() const noexcept;

        [[nodiscard]] ptr<const LoosySceneNode> asLoosy() const noexcept;

        [[nodiscard]] ptr<LoosySceneNode> asLoosy() noexcept;

        /**
         * Query if this is spartial node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if spartial node, false if not.
         */
        [[nodiscard]] bool isSpartialNode() const noexcept;

        [[nodiscard]] ptr<const SpartialSceneNode> asSpartial() const noexcept;

        [[nodiscard]] ptr<SpartialSceneNode> asSpartial() noexcept;

        /**
         * Query if this is natural node
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns True if natural node, false if not.
         */
        [[nodiscard]] bool isNaturalNode() const noexcept;

        [[nodiscard]] ptr<const NaturalSceneNode> asNatural() const noexcept;

        [[nodiscard]] ptr<NaturalSceneNode> asNatural() noexcept;

    public:
        /**
         * Query if this current node is leaf in graph
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns True if leaf, false if not.
         */
        [[nodiscard]] bool isLeaf() const noexcept;

        /**
         * Gets the max depth from current node in graph
         *
         * @author Julius
         * @date 01.08.2021
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 depth() const noexcept;

    protected:
        // TODO: Recheck
        [[deprecated]] u64 _size;

    public:
        /**
         * Traces the accumulated count of elements in graph from current node
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 size() const noexcept;

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
        [[nodiscard]] bool contains(cref<SceneNodeId> id_) const noexcept;

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
        [[nodiscard]] bool contains(const_reference_type other_) const noexcept;

    public:
        /**
         * Pushes an object onto this node or sub graph
         *
         * @author Julius
         * @date 04.11.2021
         *
         * @param data_ The data to construct from.
         * @param factory_ The factory to construct nodes.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool push(_Inout_ mref<SceneNodeCreateData> data_,
            _In_ const ptr<const SceneNodeFactory> factory_);

        /**
         * Pushes an object onto this node or sub graph
         *
         * @author Julius
         * @date 04.11.2021
         *
         * @param data_ The data to construct from.
         * @param factory_ The factory to construct nodes.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool push(_In_ const ptr<SceneNodeCreateData> data_,
            _In_ const ptr<const SceneNodeFactory> factory_);

        /**
         * Pulls this
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @param  nodeId_ The identifier.
         *
         * @returns A pull_result_type.
         */
        [[nodiscard]] pull_result_type pull(cref<SceneNodeId> nodeId_) noexcept;

        /**
         * Pulls the chained
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @param  parent_ The parent.
         * @param  pos_ The position.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool pullChained(
            const_reference_type parent_,
            IN OUT ptr<SceneNodeHead> pos_
        ) noexcept;

    protected:
        math::Transformation _transformation;
        math::Bounding _bounding;

    public:
        [[nodiscard, deprecated]] ref<math::Transformation> transform() noexcept {
            return _transformation;
        }

        [[nodiscard, deprecated]] ref<math::Bounding> bounding() noexcept {
            return _bounding;
        }

    public:
        [[nodiscard]] bool intersects(const_reference_type node_) const noexcept;

        [[nodiscard]] _Success_(return == true) bool intersects(_In_ cref<SceneNodeCreateData> data_) const noexcept;

        [[nodiscard]] bool intersectsFully(const_reference_type node_) const noexcept;

        [[nodiscard]] _Success_(return == true) bool intersectsFully(
            _In_ cref<SceneNodeCreateData> data_) const noexcept;

        [[nodiscard]] bool intersectedFully(const_reference_type node_) const noexcept;

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
        friend bool operator ==(cref<SceneNode> left_, cref<SceneNode> right_) noexcept;

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
        friend bool operator !=(cref<SceneNode> left_, cref<SceneNode> right_) noexcept;
    };

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
}
