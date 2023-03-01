#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Resource/Manage/ResourceBase.hpp>

#include "__fwd.hpp"
#include "CacheResult.hpp"
#include "../Device/Device.hpp"

namespace hg::engine::gfx::cache {
    class GlobalResourceCache final {
    public:
        friend class GlobalCacheCtrl;
        friend class LocalResourceCache;

    public:
        using this_type = GlobalResourceCache;

        using cache_value_type = smr<resource::ResourceBase>;

        using query_result_type = Result<QueryResultType, cache_value_type>;
        using store_result_type = Result<StoreResultType, cache_value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 18.06.2022
         *
         * @param   device_ The device.
         */
        GlobalResourceCache(cref<sptr<Device>> device_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 18.06.2022
         */
        ~GlobalResourceCache();

    private:
        void tidy();

    protected:
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         *
         */
        ska::bytell_hash_map<asset_guid, smr<resource::ResourceBase>> _mapped;

    public:
        [[nodiscard]] bool contains(const non_owning_rptr<const assets::Asset> asset_) const noexcept;

        [[nodiscard]] query_result_type query(
            const non_owning_rptr<const assets::Asset> asset_
        ) const noexcept;

        [[nodiscard]] query_result_type query(
            cref<asset_guid> guid_
        ) const noexcept;

    public:
        store_result_type store(_In_ cref<asset_guid> guid_, _In_ mref<smr<resource::ResourceBase>> resource_);

        bool remove(_In_ cref<asset_guid> guid_);

        bool remove(_In_ cref<asset_guid> guid_, _Out_ ref<smr<resource::ResourceBase>> resource_);

        void clear();
    };
}
