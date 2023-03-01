#pragma once

/** . */
namespace hg::hash {
    /**
     * Hash combine
     *
     * @author Julius
     * @date 30.01.2020
     *
     * @param [in,out] seed_ The seed.
     * @param 		   hash_ The hash.
     */
    inline void hash_combine(size_t& seed_, size_t hash_) {
        hash_ += 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
        seed_ ^= hash_;
    }
}
