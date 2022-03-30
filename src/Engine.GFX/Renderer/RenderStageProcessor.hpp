#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Multiplexer.hpp"

namespace ember::engine::gfx {

    class RenderStageProcessor {
    public:
        using this_type = RenderStageProcessor;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        RenderStageProcessor() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        ~RenderStageProcessor() = default;

    private:
        ptr<Multiplexer> _multiplexer;

    public:
        [[nodiscard]] const ptr<const Multiplexer> multiplexer() const noexcept;
    };
}
