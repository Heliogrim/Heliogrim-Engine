#pragma once
#include "ActorComponent.hpp"

namespace ember {

    class SceneComponent :
        public ActorComponent {
    public:
        using this_type = SceneComponent;
        using underlying_type = ActorComponent;

    public:
        SceneComponent(mref<component_type_id> typeId_, mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_);

    private:
        CompactSet<ptr<SceneNodeModel>> _sceneNodeModels;

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
        [[nodiscard]] cref<CompactSet<ptr<SceneNodeModel>>> getSceneNodeModels() const noexcept;

        template <typename Selector_>
        [[nodiscard]] Vector<ptr<SceneNodeModel>> selectSceneNodeModels(mref<Selector_> selector_) const {

            Vector<ptr<SceneNodeModel>> result {};
            eachSceneNodeModel([&](const ptr<SceneNodeModel> model_) {
                if (selector_(model_)) {
                    result.push_back(model_);
                }
            });

            return result;
        }

        void addSceneNodeModel(const ptr<SceneNodeModel> model_);
    };
}
