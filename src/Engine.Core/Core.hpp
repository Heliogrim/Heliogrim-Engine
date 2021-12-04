#pragma once

#include <atomic>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Wrapper.hpp>

/**
 * Forward Declaration
 */
namespace ember::engine::scene {
    class Scene;
}

namespace ember::engine {

    class Core {
    public:
        using this_type = Core;

    public:
        Core() noexcept;

        ~Core();

    private:
        /**
         * Atomic flag to signal stopped game core for synchronization
         *
         * @details Use stopped flag, cause read operations without negation are cheaper; Expect more reads than writes
         */
        _STD atomic_flag _stopped;

    public:
        void start();

        void stop();

    private:
        ptr<scene::Scene> _currentMainScene;

    public:
        [[nodiscard]] const ptr<scene::Scene> getCurrentScene() const noexcept;
    };

}
