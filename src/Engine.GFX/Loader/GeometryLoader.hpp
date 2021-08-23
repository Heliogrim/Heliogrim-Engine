#pragma once

#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Resource/Url.hpp>
#include "../Device/Device.hpp"
#include "../Geometry/Geometry.hpp"

namespace ember::engine::gfx::loader {
    class GeometryLoader {
    public:
        using this_type = _STD unique_ptr<GeometryLoader>;
        using const_reference_type = const GeometryLoader&;

        using future_type = concurrent::future<Geometry>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.08.2020
         */
        ~GeometryLoader() = default;

        /**
         * Makes a instance of the GeometryLoader
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
         * Gets the unique instance of GeometryLoader
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @returns A const_reference_type to the unique instance.
         */
        static const_reference_type get();

        /**
         * Loads the data structure of the given Url
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @param  url_ The Url mapping the data to load.
         *
         * @returns A future_type.
         */
        future_type load(const Url& url_) const;

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
        GeometryLoader(cref<sptr<Device>> device_);
    };
}
