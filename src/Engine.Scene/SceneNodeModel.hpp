#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Reflect/EmberReflect.hpp>

/**
 * Forward Declaration
 */
namespace ember {
    class SceneComponent;
}

namespace ember::engine::scene {
    class Scene;
}

namespace ember::engine::scene {
    class __declspec(novtable) SceneNodeModel :
        public EmberObject {
    public:
        SceneNodeModel() = delete;

        SceneNodeModel(const ptr<SceneComponent> owner_);

        virtual ~SceneNodeModel() noexcept;

    protected:
        ptr<SceneComponent> _owner;

    public:
        [[nodiscard]] const ptr<SceneComponent> owner() const noexcept;

    protected:
        math::Bounding _boundary;

    public:
        [[nodiscard]] cref<math::Bounding> getBoundaries() const noexcept;

    public:
        virtual void create(const ptr<Scene> scene_/* const ptr<SceneComponent> origin_ */) = 0;

        virtual void update(const ptr<Scene> scene_/* const ptr<SceneComponent> origin_ */) = 0;

        virtual void destroy(const ptr<Scene> scene_) = 0;

    private:
        _STD atomic_flag _markAsDeleted;

    public:
        /**
         * Check whether this model is marked for deletion
         *
         * @author Julius
         * @date 09.07.2022
         *
         * @returns True if this model should be deleted, otherwise false.
         */
        [[nodiscard]] bool markedAsDeleted() const noexcept;

        /**
         * Signal this model to be deleted
         *
         * @author Julius
         * @date 09.07.2022
         *
         * @returns True if this was already marked or deleted, otherwise false.
         */
        [[nodiscard]] bool markAsDeleted() noexcept;
    };
}
