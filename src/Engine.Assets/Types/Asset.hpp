#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Reflect/EmberReflect.hpp>
#include <Engine.Serialization/Layout/__fwd.hpp>

#include "../AssetGuid.hpp"
#include "../AssetTypeId.hpp"

namespace ember::engine::assets {
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

    class Asset :
        public EmberObject {
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
         * @param  type_ The type.
         */
        Asset(cref<asset_guid> guid_, cref<asset_type_id> type_);

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

    protected:
        /**
         * Type Identifier
         */
        asset_type_id _type;

    public:
        /**
         * Gets type identifier
         *
         * @author Julius
         * @date 03.10.2021
         *
         * @returns The type identifier.
         */
        [[nodiscard]] asset_type_id getTypeId() const noexcept;

    protected:
        /**
         * Asset Name
         */
        string _assetName;

    public:
        [[nodiscard]] string_view getAssetName() const noexcept;

        void setAssetName(string_view assetName_);
    };
}
