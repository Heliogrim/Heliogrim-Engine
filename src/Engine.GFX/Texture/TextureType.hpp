#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx {

    /**
     * Values that represent types of Textures
     *
     * [2d] : 2D Texture {Texel}
     *
     *	-> contains width and height, depth will always be 1
     *	-> does not contain multilayer
     *
     * [2d Array] : 2D Texture Array {Texel}
     *
     *	-> contains constant width and height, depth will always be &gte 1
     *	-> does contain multilayer
     *
     * [3d] : 3D Texture {Voxel}
     *
     * [Cube] : Cubemap Texture
     *
     *	-> contains width and height, depth will always be 6
     *	-> does contain multilayer [6]
     *
     *	-> TODO: enum [ Top, Left, Right, Front, Bottom, Back ]
     * 
     */
    enum class TextureType: u8 {
        eUndefined = 0b0,

        // Planar Textures
        e2d = 0b1 << 0,
        e2dArray = 0b1 << 1,

        // Volumetric Texture
        e3d = 0b1 << 4,
        eCube = 0b1 << 5
    };

    typedef Flag<TextureType> TextureTypes;
}
