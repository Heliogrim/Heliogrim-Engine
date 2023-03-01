#pragma once

#include "Inbuilt.hpp"

namespace hg {
    /**
     * Forward Declaration
     */
    class ActorComponent;
    class SceneComponent;

    class ComponentHierarchy;

    class __declspec(novtable) IComponentRegisterContext {
    public:
        virtual void add(const ptr<ActorComponent> component_) = 0;

        virtual void add(cref<ComponentHierarchy> hierarchy_) = 0;
    };
}
