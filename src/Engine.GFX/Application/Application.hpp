#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include "../vkinc.hpp"

namespace hg::engine::gfx {
    class Application {
    public:
        using value_type = Application;
        using reference_type = Application&;
        using const_reference_type = const Application&;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        Application() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Application();

        /**
         * Setups this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void setup();

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void destroy();

        /**
         * Member dereference operator
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns The dereferenced object.
         */
        vk::Instance* operator->();

        const ptr<const vk::Instance> operator->() const noexcept;

        /**
         * Instance casting operator
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns The result of the operation.
         */
        operator vk::Instance() const;

    private:
        vk::Instance _instance;

        static Vector<const char*> _validation;

        /**
         * Get available extensions for api driver
         *
         * @author Julius
         * @date 09.11.2020
         */
        static Vector<const char*> getExtensions();
    };
}
