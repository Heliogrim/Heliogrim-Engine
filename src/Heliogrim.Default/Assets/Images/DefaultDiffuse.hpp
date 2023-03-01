#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DefaultDiffuse :
        public ImageAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        inline static constexpr type_id typeId { "image::DefaultDiffuse"_typeId };

    public:
        DefaultDiffuse():
            ImageAsset(
                DefaultDiffuse::auto_guid(),
                R"(resources\imports\ktx\default_diffuse.ktx)"
            ) {}
    };
}
