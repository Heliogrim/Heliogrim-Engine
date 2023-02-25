#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconTodo :
        public ImageAsset,
        public AssetAutoGuid<IconTodo> {
    public:
        IconTodo():
            ImageAsset(
                IconTodo::unstable_auto_guid(),
                R"(resources\imports\ktx\todo.ktx)"
            ) {}
    };
}
