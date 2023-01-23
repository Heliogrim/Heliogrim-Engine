#pragma once

#include "Loader.hpp"
#include "../Stream/Streamable.hpp"

namespace ember::engine::resource {
    template <assets::IsStreamableAsset AssetType_>
    struct StreamLoaderOptions;

    template <assets::IsStreamableAsset AssetType_>
    class __declspec(novtable) StreamLoader :
        public Loader<AssetType_, partial::Streamable> {
    public:
        using this_type = StreamLoader<AssetType_>;
        using underlying_type = Loader<AssetType_, partial::Streamable>;
        using base_type = LoaderBase;

        using underlying_type::value_type;
        using underlying_type::reference_type;
        using underlying_type::const_reference_type;

        using underlying_type::result_type;
        using underlying_type::options_type;

        using stream_options_type = ptr<StreamLoaderOptions<AssetType_>>;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 18.07.2022
         */
        StreamLoader() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 18.07.2022
         */
        ~StreamLoader() override = default;

    public:
        /**
         * Stream loading operation on given resource_ based on provided meta information
         *
         * @author Julius
         * @date 18.07.2022
         *
         * @param resource_ The resource where to apply stream data changes (loading/allocating) and apply meta information.
         * @param options_  The meta information to mutate resource.
         */
        virtual void streamLoad(
            _Inout_ const ptr<partial::Streamable<ResourceBase>> resource_,
            stream_options_type options_
        ) = 0;

        /**
         * Stream unloading operation on given resource_ based on provided meta information
         *
         * @author Julius
         * @date 18.07.2022
         *
         * @param resource_ The resource where to apply stream data changes (unloading/freeing) and apply meta information.
         * @param options_  The meta information to mutate resource.
         */
        virtual void streamUnload(
            _Inout_ const ptr<partial::Streamable<ResourceBase>> resource_,
            stream_options_type options_
        ) = 0;
    };
}
