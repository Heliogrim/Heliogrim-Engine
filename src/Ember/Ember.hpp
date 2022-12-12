#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "AssetDatabase.hpp"
#include "Session.hpp"

namespace ember::engine {
    class Engine;
}

namespace ember {
    class Ember {
    public:
        static void instantiate();

        static void destroy();

    protected:
        static uptr<engine::Engine> _engine;

    public:
        [[nodiscard]] static const non_owning_rptr<engine::Engine> getEngine() noexcept;

    public:
        [[nodiscard]] static Session getSession();

        [[nodiscard]] static Session getSession(std::nothrow_t) noexcept;

    public:
        /**
         * Gets the Assets Module Interface
         *
         * @author Julius
         * @date 27.09.2021
         *
         * @returns An AssetDatabase.
         */
        [[nodiscard]] static AssetDatabase assets() noexcept;
    };
}
