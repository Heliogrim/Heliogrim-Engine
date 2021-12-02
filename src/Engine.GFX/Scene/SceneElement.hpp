#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../GraphicPassMask.hpp"
#include "../ModelBatch.hpp"
#include "../ModelState.hpp"

namespace ember::engine::proxy {
    /**
     * Forward Declaration
     */
    class SceneProxiedRef;
}

namespace ember::engine::gfx {

    class SceneElement {
    private:
        /**
         * SceneProxiedRef / Owner
         */
        wptr<proxy::SceneProxiedRef> _owner;

    public:
        /**
         * Gets the owner
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ptr&lt;proxy::SceneProxiedRef&gt;
         */
        [[nodiscard]] wptr<proxy::SceneProxiedRef> owner() const noexcept;

    private:
        /**
         * Batches
         */
        Vector<ModelBatch> _batches;

    public:
        /**
         * Query for stored immutable batch by given GraphicPassMask
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  mask_ The RenderPassMask
         *
         * @returns A cref&lt;ember::engine::gfx::ModelBatch&gt;
         */
        [[nodiscard]] cref<ModelBatch> batch(cref<GraphicPassMask> mask_) const;

        /**
         * Query for stored batch by given GraphicPassMask
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  mask_ The RenderPassMask
         *
         * @returns A ref&lt;ember::engine::gfx::ModelBatch&gt;
         */
        [[nodiscard]] ref<ModelBatch> batch(cref<GraphicPassMask> mask_);

    private:
        /**
         * Model State
         */
        ptr<IModelState> _state;

    public:
        /**
         * Query for stored immutable ModelState
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns A ptr<const gfx::IModelState>
         */
        [[nodiscard]] ptr<const IModelState> state() const noexcept;

        /**
         * Query for stored mutable ModelState
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns A ptr<gfx::IModelState>
         */
        [[nodiscard]] ptr<IModelState> state() noexcept;
    };
}
