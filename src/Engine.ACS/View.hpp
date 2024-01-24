#pragma once
#include "Component.hpp"
#include "Pool.hpp"

namespace hg::engine::acs {
    /**
     * A view.
     *
     * @author Julius
     * @date 05.09.2020
     */
    template <typename...>
    class view;

    template <typename Entity, typename... Components>
    class view<Entity, Components...> { };

    template <typename Entity, typename Component>
    class view<Entity, Component> {
        using pool_type = Pool<Component>;

    public:
        using pool_type = Pool<Component>;
        using size_type = typename pool_type::size_type;

        using raw_type = Component;
        using entity_type = Entity;

        using value_type = view<Entity, Component>;
        using reference_type = view<Entity, Component>&;
        using const_reference_type = const view<Entity, Component>&;

        using iterator = typename Pool<Component>::iterator;
        using const_iterator = typename Pool<Component>::const_iterator;

        using reverse_iterator = typename Pool<Component>::reverse_iterator;
        using const_reverse_iterator = typename Pool<Component>::const_reverse_iterator;

        view(typename pool_type::reference_type pool_) noexcept :
            _pool(&pool_) {}

        [[nodiscard]] size_type size() const noexcept {
            return _pool->size();
        }

        [[nodiscard]] bool empty() const noexcept {
            return _pool->empty();
        }

        [[nodiscard]] iterator begin() noexcept {
            return _pool->storage<Entity>().begin();
        }

        [[nodiscard]] const_iterator cbegin() const noexcept {
            return _pool->storage<Entity>().cbegin();
        }

        [[nodiscard]] iterator end() noexcept {
            return _pool->storage<Entity>().end();
        }

        [[nodiscard]] const_iterator cend() const noexcept {
            return _pool->storage<Entity>().cend();
        }

        [[nodiscard]] reverse_iterator rbegin() noexcept {
            return _pool->storage<Entity>().rbegin();
        }

        [[nodiscard]] const_reverse_iterator rcbegin() const noexcept {
            return _pool->storage<Entity>().rcbegin();
        }

        [[nodiscard]] reverse_iterator rend() noexcept {
            return _pool->storage<Entity>().rend();
        }

        [[nodiscard]] const_reverse_iterator rcend() const noexcept {
            return _pool->storage<Entity>().rcend();
        }

        [[nodiscard]] raw_type front() const {
            const auto it = cbegin();
            return it != cend() ? *it : null_component;
        }

        [[nodiscard]] raw_type back() const {
            const auto it = rcbegin();
            return it != rcend() ? *it : null_component;
        }

        [[nodiscard]] raw_type operator[](const size_type& index_) const {
            return cbegin()[index_];
        }

        void each(const std::function<void(const Component&)>& fnc_) const {
            for (const auto entry : *this) {
                fnc_(entry);
            }
        }

        void map(const std::function<Component&&(Component&&)>& fnc_) {
            for (auto&& entry : *this) {
                entry = std::move(fnc_(std::move(entry)));
            }
        }

    private:
        pool_type* _pool;
    };

    template <typename Component>
    class view<Component> {
    public:
        using pool_type = Pool<Component>;
        using size_type = typename pool_type::size_type;

        using raw_type = Component;

        using value_type = view<Component>;
        using reference_type = view<Component>&;
        using const_reference_type = const view<Component>&;

        using iterator = typename pool_type::iterator;
        using const_iterator = typename pool_type::const_iterator;

        using reverse_iterator = typename pool_type::reverse_iterator;
        using const_reverse_iterator = typename pool_type::const_reverse_iterator;

        view(typename pool_type::reference_type pool_) noexcept :
            _pool(&pool_) {}

        [[nodiscard]] size_type size() const noexcept {
            return _pool->size();
        }

        [[nodiscard]] bool empty() const noexcept {
            return _pool->empty();
        }

        [[nodiscard]] iterator begin() noexcept {
            return _pool->storage().begin();
        }

        [[nodiscard]] const_iterator cbegin() const noexcept {
            return _pool->storage().cbegin();
        }

        [[nodiscard]] iterator end() noexcept {
            return _pool->storage().end();
        }

        [[nodiscard]] const_iterator cend() const noexcept {
            return _pool->storage().cend();
        }

        [[nodiscard]] reverse_iterator rbegin() noexcept {
            return _pool->storage().rbegin();
        }

        [[nodiscard]] const_reverse_iterator rcbegin() const noexcept {
            return _pool->storage().rcbegin();
        }

        [[nodiscard]] reverse_iterator rend() noexcept {
            return _pool->storage().rend();
        }

        [[nodiscard]] const_reverse_iterator rcend() const noexcept {
            return _pool->storage().rcend();
        }

        [[nodiscard]] raw_type front() const {
            const auto it = cbegin();
            return it != cend() ? *it : null_component;
        }

        [[nodiscard]] raw_type back() const {
            const auto it = rcbegin();
            return it != rcend() ? *it : null_component;
        }

        [[nodiscard]] raw_type operator[](const size_type& index_) const {
            return cbegin()[index_];
        }

        void each(const std::function<void(const Component&)>& fnc_) const {
            for (const auto entry : *this) {
                fnc_(entry);
            }
        }

        void map(const std::function<Component&&(Component&&)>& fnc_) {
            for (auto&& entry : *this) {
                entry = std::move(fnc_(std::move(entry)));
            }
        }

    private:
        pool_type* _pool;
    };
}
