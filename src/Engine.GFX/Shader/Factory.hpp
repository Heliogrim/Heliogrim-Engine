#pragma once

#include "ShaderBindingGroup.hpp"
#include "Prototype.hpp"
#include "Shader.hpp"
#include "../Device/Device.hpp"

#include <fstream>

inline _STD vector<char> read_shader_file(const char* file_) {
    _STD vector<char> spirv {};

    _STD ifstream in {};
    in.open(file_, _STD ios::ate | _STD ios::in | _STD ios::binary);

    assert(in.is_open());

    const size_t size = in.tellg();
    spirv.resize(size, 0);

    in.seekg(0, _STD ios::beg);
    in.read(spirv.data(), size);

    in.close();
    return spirv;
}

namespace ember::engine::gfx::shader {

    struct FactoryBuildResult {
        Vector<ShaderBindingGroup> groups;
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
