#pragma once

#include "../Types.hpp"
#include "../__macro.hpp"

namespace ember::hash {

    /**
     * Murmur Hash 3 (x86/64 Platform) (32Bit Output)
     *
     * @author Julius
     * @date 12.11.2020
     *
     * @param 		   data_ The data.
     * @param 		   length_ The length.
     * @param 		   seed_ The seed.
     * @param [in,out] dst_ If non-null, destination for the.
     */
    void murmur3_x86_32(IN const void* RESTRICT data_, IN const u32 length_, IN const u32 seed_,
        OUT u32* RESTRICT dst_);

    /**
     * Murmur Hash 3 (x64 Platform) (64Bit Output)
     *
     * @author Julius
     * @date 12.11.2020
     *
     * @param          data_ The data.
     * @param          length_ The length.
     * @param          seed_ The seed.
     * @param [in,out] dst_ If non-null, destination for the.
     */
    void murmur3_x64_64(IN const void* RESTRICT data_, IN const u32 length_, IN const u32 seed_,
        OUT u64* RESTRICT dst_);

    /**
     * Murmur Hash 3 (x64 Platform) (128Bit Output)
     *
     * @author Julius
     * @date 12.11.2020
     *
     * @param 		   data_ The data.
     * @param 		   length_ The length.
     * @param 		   seed_ The seed.
     * @param [in,out] dst_ If non-null, destination for the.
     */
    void murmur3_x64_128(IN const void* RESTRICT data_, IN const u32 length_, IN const u32 seed_,
        OUT void* RESTRICT dst_);

}
