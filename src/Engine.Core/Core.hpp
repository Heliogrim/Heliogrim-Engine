#pragma once

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

    public:
        void start();

        void stop();

    private:
        ptr<scene::Scene> _currentMainScene;

    public:
        [[nodiscard]] const ptr<scene::Scene> getCurrentScene() const noexcept;
    };

}
