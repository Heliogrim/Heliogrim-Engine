#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::proxy {

    /**
     * Forward Declaration
     */
    class SceneProxy;

    /**
     * Forward Declaration
     */
    struct IModelState;

    /**
     * Forward Declaration
     */
    struct SceneComponent;

    struct StatedSceneProxy {
        ptr<const SceneProxy> proxy;
        ptr<const IModelState> state;

        /**
         * Transit the given state to a clone of the immutable internal stored state.
         * 
         *  Will return a new cloned state, if changes happen,
         *  otherwise will return pointer to already stored internal state
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  obj_ The source component to transit state from
         *
         * @returns A pair&lt;bool,ptr&lt;const IModelState&gt;&gt;
         */
        _STD pair<bool, ptr<const IModelState>> transit(ptr<const SceneComponent> obj_) const;
    };
}
