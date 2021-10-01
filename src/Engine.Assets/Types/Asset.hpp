#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

#include "../AssetGuid.hpp"
#include "../AssetTypeId.hpp"

namespace ember {

    /*
                                Level   Entity	    Serialized<Component>	GfxMaterial	PfxMaterial	SfxMaterial	Texture     Image	    Sound	    Geometry
        Level	                -	    1 :: *      T 1 :: *	            T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *
        Entity	                -	    -	        1 :: *                  T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *	T 1 :: *
        Serialized<Component>	-	    -	        -	                    0,1 :: 0,*	0,1 :: 0,*	0,1 :: 0,*	T 1 :: *	T 1 :: *	0,1 :: 0,*	0,1 :: 0,*
        GfxMaterial	            -	    -	        -	                    -	        -	        -	        1 :: *	    T 1 :: *	-	        -
        PfxMaterial	            -	    -	        -	                    -	        -	        -	        - (?)	    - (T?)      -	        -
        SfxMaterial	            -	    -	        -	                    -	        -	        -	        -	        -	        -	        -
        Texture	                -	    -	        -	                    -	        -	        -	        -	        1 :: 0,1	-	        -
        Image	                -	    -	        -	                    -	        -	        -	        -	        -	        -	        -
        Sound	                -	    -	        -	                    -	        -	        -	        -	        -	        -	        -
        Geometry                -	    -	        -	                    -	        -	        -	        -	        -	        -	        -
     */

    /**
     * The Concept for assets
     */
    template <class Ty>
    concept IsAsset = requires(const Ty obj) {
        { obj.get_guid() } -> std::same_as<asset_guid>;
    } && ember::HasType<Ty, asset_type_id>;

    class Asset {
    public:
        using value_type = Asset;
        using reference_type = Asset&;
        using const_reference_type = const Asset&;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @param  guid_ Unique identifier.
         */
        Asset(cref<asset_guid> guid_);

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 09.01.2021
         */
        ~Asset() noexcept;

    protected:
        /**
         * Guid
         */
        asset_guid _guid;

    public:
        /**
         * Gets the unique identifier
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns The unique identifier.
         */
        [[nodiscard]] asset_guid get_guid() const noexcept;
    };
}
