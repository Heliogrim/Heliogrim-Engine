#pragma once
#include "SceneGraph.hpp"
#include "../Node/SceneNodeFactory.hpp"

namespace ember::engine::scene {

    template <class PayloadType_>
    class MutableSceneGraph :
        public SceneGraph<PayloadType_> {
    public:
        using this_type = MutableSceneGraph<PayloadType_>;
        using underlying_type = SceneGraph<PayloadType_>;

        using factory_type = SceneNodeFactory<
            typename underlying_type::node_storage_type,
            typename underlying_type::element_storage_type
        >;

    protected:
        [[nodiscard]] factory_type factory() const noexcept {
            return factory_type {
                underlying_type::_nodeStorage.get(),
                underlying_type::_elementStorage.get()
            };
        }

    public:
        bool push(ptr<PayloadType_> element_) {
            throw NotImplementedException();
        }

        bool push(ptr<typename underlying_type::node_type> node_) {
            throw NotImplementedException();
        }

        bool insert(ptr<PayloadType_> element_, const ptr<typename underlying_type::node_type> where_) {
            throw NotImplementedException();
        }

        bool insert(ptr<typename underlying_type::node_type> node_,
            const ptr<typename underlying_type::node_type> where_) {
            throw NotImplementedException();
        }

        bool pop(const ptr<PayloadType_> element_) {
            throw NotImplementedException();
        }

        bool pop(const ptr<typename underlying_type::node_type> node_) {
            throw NotImplementedException();
        }

        bool erase(ref<ptr<typename underlying_type::node_type>> subGraph_) {
            throw NotImplementedException();
        }
    };

}
