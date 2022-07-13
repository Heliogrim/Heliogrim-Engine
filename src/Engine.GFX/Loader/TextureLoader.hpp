#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <gli/gli.hpp>

#include "../Cache/__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Texture/__fwd.hpp"
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

        // [[nodiscard]] ember::concurrent::promise<uptr<VirtualTextureView>> loadTo(const Url& url_, mref<uptr<VirtualTextureView>> dst_) const;
        [[nodiscard]] uptr<VirtualTextureView> loadTo(const Url& url_, mref<uptr<VirtualTextureView>> dst_) const;

        Texture __tmp__load(const Url& url_);

    protected:
        ptr<cache::GlobalCacheCtrl> _cacheCtrl;

        /**
         * Constructor
         *
         * @author Julius
         * @date 23.08.2020
         *
         * @param  cache_ The resource cache to use.
         */
        TextureLoader(const ptr<cache::GlobalCacheCtrl> cache_);
    };
}
