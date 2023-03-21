#pragma once

namespace hg::engine::assets {
    class Asset;

    /**/

    class IAssetRegistry;
    class AssetRegistry;
}

namespace hg::engine::assets::system {
    struct AssetDescriptor;
    class AssetFilter;
    class AssetRepository;
    class InMemAssetRepository;

    /**/

    template <typename>
    struct IndexTrait;

    template <typename>
    class AutoIndexTable;
    template <typename>
    class IndexTable;
    class IndexTableBase;
}
