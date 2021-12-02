#pragma once

#include "BindingGroup.hpp"
#include "Prototype.hpp"
#include "Shader.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx::shader {

    struct FactoryBuildResult {
        Vector<BindingGroup> groups;
        Vector<ptr<Shader>> shaders;
    };

    class Factory {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @param  device_ The device.
         */
        Factory(cref<sptr<Device>> device_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 27.11.2020
         */
        ~Factory() noexcept;

        /**
         * Builds the given prototype list to shaders and binding groups
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  list_ The list.
         *
         * @returns A FactoryBuildResult.
         */
        [[nodiscard]] FactoryBuildResult build(_STD initializer_list<Prototype> list_) const;

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    public:
        /**
         * Gets the device
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;
    };
}
