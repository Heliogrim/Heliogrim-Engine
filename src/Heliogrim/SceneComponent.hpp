#pragma once
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Scene/SceneNodeModel.hpp>

#include "ActorComponent.hpp"

namespace hg {
    class SceneComponent :
        public ActorComponent {
    public:
        using this_type = SceneComponent;
        using underlying_type = ActorComponent;

        using SceneNodeModel = ::hg::engine::scene::SceneNodeModel;

    public:
        SceneComponent(
            mref<component_type_id> typeId_,
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );

        // Warning: does not propagate currently
        ~SceneComponent();

    private:
        CompactSet<non_owning_rptr<SceneNodeModel>> _sceneNodeModels;

    private:
        template <typename Fnc_>
        void eachSceneNodeModel(mref<Fnc_> fnc_) const {
            /**
             *
             */
            for (auto& entry : _sceneNodeModels) {
                fnc_(entry);
            }
        }

    public:
        [[nodiscard]] cref<CompactSet<non_owning_rptr<SceneNodeModel>>> getSceneNodeModels() const noexcept;

        template <typename Selector_>
        [[nodiscard]] Vector<non_owning_rptr<SceneNodeModel>> selectSceneNodeModels(mref<Selector_> selector_) const {
            Vector<non_owning_rptr<SceneNodeModel>> result {};
            eachSceneNodeModel(
                [&](const non_owning_rptr<SceneNodeModel> model_) {
                    if (selector_(model_)) {
                        result.push_back(model_);
                    }
                }
            );

            return result;
        }

        void addSceneNodeModel(const non_owning_rptr<SceneNodeModel> model_);
    };
}
