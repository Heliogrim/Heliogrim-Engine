#pragma once

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Resource/Url.hpp>
#include <gli/gli.hpp>

#include "../Device/Device.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::loader {
    class TextureLoader {
    public:
        using value_type = TextureLoader;
        using reference_type = TextureLoader&;
        using const_reference_type = const TextureLoader&;

        using this_type = _STD unique_ptr<TextureLoader>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.08.2020
         */
        ~TextureLoader() = default;

        /**
         * Makes a instance of the TextureLoader
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @param  device_ The device.
         *
         * @returns A const_reference_type to the unique instance.
         */
        static const_reference_type make(cref<sptr<Device>> device_);

        /**
         * Gets the unique instance of TextureLoader
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @returns A const_reference_type to the unique instance.
         */
        static const_reference_type get();

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 20.12.2020
         */
        static void destroy();

        /**
         * Loads the given URL
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @param  url_ The URL to load.
         *
         * @returns A concurrent::promise&lt;Texture&gt;
         */
        ember::concurrent::promise<Texture> load(const Url& url_) const;

    private:
        static this_type _this;
        sptr<Device> _device;

        /**
         * Constructor
         *
         * @author Julius
         * @date 23.08.2020
         *
         * @param  device_ The device.
         */
        TextureLoader(cref<sptr<Device>> device_);
    };
}
