#pragma once

#include <Ember/Ember.hpp>

#include "Assets/GfxMaterials/ForestGround01.hpp"
#include "Assets/GfxMaterials/GrassWild01.hpp"
#include "Assets/Images/ForestGround01Albedo.hpp"
#include "Assets/Images/ForestGround01AO.hpp"
#include "Assets/Images/ForestGround01Cavity.hpp"
#include "Assets/Images/ForestGround01Displacement.hpp"
#include "Assets/Images/ForestGround01Gloss.hpp"
#include "Assets/Images/ForestGround01Normal.hpp"
#include "Assets/Images/ForestGround01Roughness.hpp"
#include "Assets/Images/ForestGround01Specular.hpp"
#include "Assets/Images/GrassWild01Albedo.hpp"
#include "Assets/Images/GrassWild01AO.hpp"
#include "Assets/Images/GrassWild01Cavity.hpp"
#include "Assets/Images/GrassWild01Displacement.hpp"
#include "Assets/Images/GrassWild01Gloss.hpp"
#include "Assets/Images/GrassWild01Normal.hpp"
#include "Assets/Images/GrassWild01Roughness.hpp"
#include "Assets/Images/GrassWild01Specular.hpp"
#include "Assets/Textures/ForestGound01Albedo.hpp"
#include "Assets/Textures/ForestGound01AO.hpp"
#include "Assets/Textures/ForestGound01Cavity.hpp"
#include "Assets/Textures/ForestGound01Displacement.hpp"
#include "Assets/Textures/ForestGound01Gloss.hpp"
#include "Assets/Textures/ForestGound01Normal.hpp"
#include "Assets/Textures/ForestGound01Roughness.hpp"
#include "Assets/Textures/ForestGound01Specular.hpp"
#include "Assets/Textures/GrassWild01Albedo.hpp"
#include "Assets/Textures/GrassWild01AO.hpp"
#include "Assets/Textures/GrassWild01Cavity.hpp"
#include "Assets/Textures/GrassWild01Displacement.hpp"
#include "Assets/Textures/GrassWild01Gloss.hpp"
#include "Assets/Textures/GrassWild01Normal.hpp"
#include "Assets/Textures/GrassWild01Roughness.hpp"
#include "Assets/Textures/GrassWild01Specular.hpp"

using namespace ember;

inline void push() {

    //
    auto wdb = Ember::assets();

    //
    using namespace ember::game::assets;

    //
    wdb.insert(new image::ForestGround01Albedo {});
    wdb.insert(new image::ForestGround01AO {});
    wdb.insert(new image::ForestGround01Cavity {});
    wdb.insert(new image::ForestGround01Displacement {});
    wdb.insert(new image::ForestGround01Gloss {});
    wdb.insert(new image::ForestGround01Normal {});
    wdb.insert(new image::ForestGround01Roughness {});
    wdb.insert(new image::ForestGround01Specular {});

    wdb.insert(new image::GrassWild01Albedo {});
    wdb.insert(new image::GrassWild01AO {});
    wdb.insert(new image::GrassWild01Cavity {});
    wdb.insert(new image::GrassWild01Displacement {});
    wdb.insert(new image::GrassWild01Gloss {});
    wdb.insert(new image::GrassWild01Normal {});
    wdb.insert(new image::GrassWild01Roughness {});
    wdb.insert(new image::GrassWild01Specular {});

    wdb.insert(new texture::ForestGround01Albedo {});
    wdb.insert(new texture::ForestGround01AO {});
    wdb.insert(new texture::ForestGround01Cavity {});
    wdb.insert(new texture::ForestGround01Displacement {});
    wdb.insert(new texture::ForestGround01Gloss {});
    wdb.insert(new texture::ForestGround01Normal {});
    wdb.insert(new texture::ForestGround01Roughness {});
    wdb.insert(new texture::ForestGround01Specular {});

    wdb.insert(new texture::GrassWild01Albedo {});
    wdb.insert(new texture::GrassWild01AO {});
    wdb.insert(new texture::GrassWild01Cavity {});
    wdb.insert(new texture::GrassWild01Displacement {});
    wdb.insert(new texture::GrassWild01Gloss {});
    wdb.insert(new texture::GrassWild01Normal {});
    wdb.insert(new texture::GrassWild01Roughness {});
    wdb.insert(new texture::GrassWild01Specular {});

    wdb.insert(new material::ForestGround01 {});
    wdb.insert(new material::GrassWild01 {});
}
