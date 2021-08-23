#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "SceneNodeId.hpp"
#include "../SceneTraits.hpp"

#include "SceneNodeHead.hpp"

namespace ember::engine::scene {

    class SceneNodeHeadContainer {
    public:
        using type_traits = scene_node_shared_traits;

        using value_type = SceneNodeHead;
        using pointer_type = ptr<value_type>;
        using value_id_type = SceneNodeId;

        using size_type = type_traits::container_size_type;
        using difference_type = type_traits::container_diff_type;

    private:
        using allocator_base = std::allocator<value_type>;
        using allocator_type = std::_Rebind_alloc_t<allocator_base, value_type>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        /** The allocate */
        allocator_type _alloc;

        value_type _base[type_traits::max_nodes_per_layer];
        pointer_type _last = nullptr;

    public:
        /**
         * Unsafe direct access to memory base
         *
         * @author Julius
         * @date 30.07.2021
         *
         * @returns A pointer_type.
         */
        [[nodiscard]] pointer_type unsafe_base() const noexcept;

        /**
         * Unsafe direct access to memory last
         *
         * @author Julius
         * @date 30.07.2021
         *
         * @returns A pointer_type.
         */
        [[nodiscard]] pointer_type unsafe_last() const noexcept;

    private:
        /**
         * Gets the allocator
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @returns The allocate.
         */
        [[nodiscard]] constexpr cref<allocator_type> getAlloc() const noexcept;

        /**
         * Gets the allocator
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @returns The allocate.
         */
        [[nodiscard]] constexpr ref<allocator_type> getAlloc() noexcept;

        /**
         * Destroys values in sequence
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @param  from_ Source for the.
         * @param  to_ to.
         */
        constexpr void destroy(pointer_type from_, pointer_type to_);

        /**
         * Tidy up object
         *
         * @author Julius
         * @date 19.07.2021
         */
        constexpr void tidy() noexcept;

    public:
        template <bool Const>
        class scene_node_container_iterator final {
            friend class SceneNodeHeadContainer;

        public:
            using iterator_category = _STD contiguous_iterator_tag;
            using difference_type = type_traits::container_diff_type;
            using value_type = _STD conditional_t<Const, const SceneNodeHead, SceneNodeHead>;

            using this_type = scene_node_container_iterator<Const>;
            using reference_type = ref<this_type>;
            using const_reference_type = cref<this_type>;
            using pointer_type = ptr<value_type>;

        protected:
            pointer_type _ptr;

        public:
            /**
             * Default constructor
             *
             * @author Julius
             * @date 13.07.2021
             */
            scene_node_container_iterator() noexcept :
                _ptr(nullptr) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  ptr_ The pointer.
             */
            scene_node_container_iterator(pointer_type ptr_) noexcept :
                _ptr(ptr_) {}

            /**
             * Copy Constructor
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  other_ The other.
             */
            scene_node_container_iterator(cref<this_type> other_) :
                _ptr(other_._ptr) {}

            /**
             * Move Constructor
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  other_ The other.
             */
            scene_node_container_iterator(mref<this_type> other_) noexcept :
                _ptr(_STD exchange(other_._ptr, nullptr)) {}

            /**
             * Copy Assignment operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            reference_type operator=(cref<this_type> other_) {
                if (&other_ != this) {
                    _ptr = other_._ptr;
                }

                return *this;
            }

            /**
             * Move Assignment operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            reference_type operator=(mref<this_type> other_) noexcept {
                if (&other_ != this) {
                    _ptr = _STD exchange(other_._ptr, nullptr);
                }

                return *this;
            }

            /**
             * Gets the value
             *
             * @returns A ref&lt;value_type&gt;
             */
            [[nodiscard]] ref<value_type> value() const {
                return *_ptr;
            }

            /**
             * Gets the pointer
             *
             * @returns A ptr&lt;value_type&gt;
             */
            [[nodiscard]] pointer_type pointer() const noexcept {
                return _ptr;
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] ref<value_type> operator*() const {
                return *_ptr;
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @returns The dereferenced object.
             */
            ptr<value_type> operator->() const {
                return _ptr;
            }

            /**
             * Array indexer operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  diff_ The difference.
             *
             * @returns The indexed value.
             */
            [[nodiscard]] ref<value_type> operator[](difference_type diff_) const {
                return *(_ptr + diff_);
            }

            /**
             * Pre-Increment operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator++() {
                ++_ptr;
                return *this;
            }

            /**
             * Post-Increment operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator++(int) {
                this_type tmp { *this };
                ++(*this);
                return tmp;
            }

            /**
             * Pre-Decrement operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator--() {
                --_ptr;
                return *this;
            }

            /**
             * Post-Decrement operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator--(int) {
                this_type tmp { *this };
                --(*this);
                return tmp;
            }

            /**
             * Advance forward operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  diff_ The difference.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] this_type operator+(difference_type diff_) const {
                return this_type { _ptr + diff_ };
            }

            /**
             * Advance backward operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  diff_ The difference.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] this_type operator-(difference_type diff_) const {
                return this_type { _ptr - diff_ };
            }

            /**
             * Advance forward assignment operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  diff_ The difference.
             *
             * @returns The result of the operation.
             */
            [[maybe_unused]] reference_type operator+=(difference_type diff_) {
                _ptr += diff_;
                return *this;
            }

            /**
             * Advance backward assignment operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  diff_ The difference.
             *
             * @returns The result of the operation.
             */
            [[maybe_unused]] reference_type operator-=(difference_type diff_) {
                _ptr -= diff_;
                return *this;
            }

            /**
             * Equality operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are considered equivalent.
             */
            constexpr friend bool operator==(const_reference_type left_, const_reference_type right_) {
                return left_._ptr == right_._ptr;
            }

            /**
             * Inequality operator
             *
             * @author Julius
             * @date 13.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are not considered equivalent.
             */
            constexpr friend bool operator!=(const_reference_type left_, const_reference_type right_) {
                return left_._ptr != right_._ptr;
            }

            /**
             * Less-than comparison operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the first parameter is less than the second.
             */
            constexpr friend bool operator<(const_reference_type left_, const_reference_type right_) {
                return left_._ptr < right_._ptr;
            }

            /**
             * Less-than-or-equal comparison operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the first parameter is less than or equal to the second.
             */
            constexpr friend bool operator<=(const_reference_type left_, const_reference_type right_) {
                return left_._ptr <= right_._ptr;
            }

            /**
             * Greater-than comparison operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the first parameter is greater than to the second.
             */
            constexpr friend bool operator>(const_reference_type left_, const_reference_type right_) {
                return left_._ptr > right_._ptr;
            }

            /**
             * Greater-than-or-equal comparison operator
             *
             * @author Julius
             * @date 28.07.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the first parameter is greater than or equal to the second.
             */
            constexpr friend bool operator>=(const_reference_type left_, const_reference_type right_) {
                return left_._ptr >= right_._ptr;
            }

            /**
             * Three-way comparison operator
             *
             * @returns The order representation
             */
            constexpr friend difference_type operator<=>(const_reference_type left_, const_reference_type right_) {
                return difference_type { left_._ptr - right_._ptr };
            }
        };

        using iterator = scene_node_container_iterator<false>;
        using const_iterator = scene_node_container_iterator<true>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 13.07.2021
         */
        SceneNodeHeadContainer() noexcept (_STD is_nothrow_constructible_v<SceneNodeHead>);

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 19.07.2021
         */
        SceneNodeHeadContainer(cref<SceneNodeHeadContainer>) = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @param other_ The other.
         */
        SceneNodeHeadContainer(mref<SceneNodeHeadContainer> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.07.2021
         */
        ~SceneNodeHeadContainer() noexcept;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @returns A shallow copy of this.
         */
        ref<SceneNodeHeadContainer> operator=(cref<SceneNodeHeadContainer>) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<SceneNodeHeadContainer> operator=(mref<SceneNodeHeadContainer> other_) noexcept;

    public:
        /**
         * Check whether container has elements
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * Gets the size
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type size() const noexcept;

        /**
         * Maximum size
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns The maximum size of the.
         */
        [[nodiscard]] constexpr size_type max_size() const noexcept;

    public:
        /**
         * Access immutable reference to indexed element
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @param  pos_ The position.
         *
         * @returns The indexed value.
         */
        [[nodiscard]] cref<value_type> operator[](size_t pos_) const;

        /**
         * Access mutable reference to indexed element
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @param  pos_ The position.
         *
         * @returns The indexed value.
         */
        [[nodiscard]] ref<value_type> operator[](size_t pos_);

    public:
        /**
         * Clears this to its blank/initial state
         *
         * @author Julius
         * @date 13.07.2021
         */
        void clear();

    private:
        /**
         * Emplaces the given payload onto this container
         *
         * @tparam Types Type of the types.
         * @param  payload_ Variable arguments providing the payload.
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <class... Types>
        bool emplace(IN mref<Types> ... payload_) {
            if (_last != &_base[type_traits::max_nodes_per_layer]) {
                allocator_traits::construct(getAlloc(), _STD _Unfancy(_last), _STD forward<Types>(payload_)...);
                ++_last;

                return true;
            }

            return false;
        }

    private:
        /**
         * Swaps internal store elements in container
         *
         * @author Julius
         * @date 01.08.2021
         *
         * @param  left_ The left.
         * @param  right_ The right.
         */
        void swap(pointer_type left_, pointer_type right_) noexcept;

    public:
        /**
         * Pushes an object onto this container
         *
         * @author Julius
         * @date 19.07.2021
         *
         * @param  node_ The node to push.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool push(IN mref<value_type> node_);

        /**
         * Pulls the last element from this container
         *
         * @author Julius
         * @date 01.08.2021
         *
         * @param  pulled_ The pulled.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool pull(OUT pointer_type pulled_) noexcept;

        /**
         * Pulls an element from container
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @param  nodeId_ Identifier for the node.
         * @param  pulled The element pulled.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool pull(cref<value_id_type> nodeId_, OUT pointer_type pulled);

        /**
         * Removes a certain element at pos_ from container
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @param  pos_ The Position to remove.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool remove(iterator pos_) noexcept;

        /**
         * Removes a sequence of elements from_ to to_
         *
         * @author Julius
         * @date 02.08.2021
         *
         * @param  from_ Source for the.
         * @param  to_ to.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool remove(iterator from_, iterator to_) noexcept;

    public:
        /**
         * Gets the begin
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator begin() const;

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin();

        /**
         * Gets the end
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator end() const;

        /**
         * Gets the end
         *
         * @author Julius
         * @date 13.07.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end();
    };
}
