#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.GFX/GraphicPassMask.hpp>
#include <Engine.GFX/ModelBatch.hpp>
#include <Engine.GFX/ModelState.hpp>

namespace ember::engine::proxy {
    /**
     * Forward Declaration
     */
    class SceneProxiedRef;
}

namespace ember::engine::scene {
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
        vector<gfx::ModelBatch> _batches;

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
        [[nodiscard]] cref<gfx::ModelBatch> batch(cref<gfx::GraphicPassMask> mask_) const;

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
        [[nodiscard]] ref<gfx::ModelBatch> batch(cref<gfx::GraphicPassMask> mask_);

    private:
        /**
         * Model State
         */
        ptr<gfx::IModelState> _state;

    public:
        /**
         * Query for stored immutable ModelState
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns A ptr<const gfx::IModelState>
         */
        [[nodiscard]] ptr<const gfx::IModelState> state() const noexcept;

        /**
         * Query for stored mutable ModelState
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns A ptr<gfx::IModelState>
         */
        [[nodiscard]] ptr<gfx::IModelState> state() noexcept;
    };
}
