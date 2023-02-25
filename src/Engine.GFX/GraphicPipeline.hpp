#pragma once
#include <Engine.Common/Wrapper.hpp>
#include "Viewport.hpp"

namespace ember::engine::gfx {
    class GraphicPipeline {
    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 01.02.2021
         */
        GraphicPipeline() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 01.02.2021
         */
        virtual ~GraphicPipeline() noexcept = default;

    protected:
        /**
         * Viewport
         */
        Viewport _viewport;

    public:
        /**
         * Gets the viewport
         *
         * @author Julius
         * @date 01.02.2021
         *
         * @returns A cref&lt;Viewport&gt;
         */
        [[nodiscard]] cref<Viewport> viewport() const noexcept;

        /**
         * Gets the viewport
         *
         * @author Julius
         * @date 01.02.2021
         *
         * @returns A ref&lt;Viewport&gt;
         */
        [[nodiscard]] ref<Viewport> viewport() noexcept;
    };
}
