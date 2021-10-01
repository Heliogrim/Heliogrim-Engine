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
    auto& db = wdb.__tmp__internal();

    //
    using namespace ember::game::assets;

    //
    db.insert(image::ForestGround01Albedo::guid, new image::ForestGround01Albedo {});
    db.insert(image::ForestGround01AO::guid, new image::ForestGround01AO {});
    db.insert(image::ForestGround01Cavity::guid, new image::ForestGround01Cavity {});
    db.insert(image::ForestGround01Displacement::guid, new image::ForestGround01Displacement {});
    db.insert(image::ForestGround01Gloss::guid, new image::ForestGround01Gloss {});
    db.insert(image::ForestGround01Normal::guid, new image::ForestGround01Normal {});
    db.insert(image::ForestGround01Roughness::guid, new image::ForestGround01Roughness {});
    db.insert(image::ForestGround01Specular::guid, new image::ForestGround01Specular {});

    db.insert(image::GrassWild01Albedo::guid, new image::GrassWild01Albedo {});
    db.insert(image::GrassWild01AO::guid, new image::GrassWild01AO {});
    db.insert(image::GrassWild01Cavity::guid, new image::GrassWild01Cavity {});
    db.insert(image::GrassWild01Displacement::guid, new image::GrassWild01Displacement {});
    db.insert(image::GrassWild01Gloss::guid, new image::GrassWild01Gloss {});
    db.insert(image::GrassWild01Normal::guid, new image::GrassWild01Normal {});
    db.insert(image::GrassWild01Roughness::guid, new image::GrassWild01Roughness {});
    db.insert(image::GrassWild01Specular::guid, new image::GrassWild01Specular {});

    db.insert(texture::ForestGround01Albedo::guid, new texture::ForestGround01Albedo {});
    db.insert(texture::ForestGround01AO::guid, new texture::ForestGround01AO {});
    db.insert(texture::ForestGround01Cavity::guid, new texture::ForestGround01Cavity {});
    db.insert(texture::ForestGround01Displacement::guid, new texture::ForestGround01Displacement {});
    db.insert(texture::ForestGround01Gloss::guid, new texture::ForestGround01Gloss {});
    db.insert(texture::ForestGround01Normal::guid, new texture::ForestGround01Normal {});
    db.insert(texture::ForestGround01Roughness::guid, new texture::ForestGround01Roughness {});
    db.insert(texture::ForestGround01Specular::guid, new texture::ForestGround01Specular {});

    db.insert(texture::GrassWild01Albedo::guid, new texture::GrassWild01Albedo {});
    db.insert(texture::GrassWild01AO::guid, new texture::GrassWild01AO {});
    db.insert(texture::GrassWild01Cavity::guid, new texture::GrassWild01Cavity {});
    db.insert(texture::GrassWild01Displacement::guid, new texture::GrassWild01Displacement {});
    db.insert(texture::GrassWild01Gloss::guid, new texture::GrassWild01Gloss {});
    db.insert(texture::GrassWild01Normal::guid, new texture::GrassWild01Normal {});
    db.insert(texture::GrassWild01Roughness::guid, new texture::GrassWild01Roughness {});
    db.insert(texture::GrassWild01Specular::guid, new texture::GrassWild01Specular {});

    db.insert(material::ForestGround01::guid, new material::ForestGround01 {});
    db.insert(material::GrassWild01::guid, new material::GrassWild01 {});
}
