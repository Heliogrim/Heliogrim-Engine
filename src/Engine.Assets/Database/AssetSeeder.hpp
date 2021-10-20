#pragma once

#include <deque>
#include <Engine.Common/Wrapper.hpp>

namespace ember {
    class Asset;
}

namespace ember::engine::assets {

    /**
     * An asset seeder.
     *
     * Warning: This class is bit of a pain in the ass, cause we need a place where we can register the function pointer to boot every user defined asset
     * Warning: The registration of the asset data and internal state representation is currently guaranteed via constructor of ´ember::Asset` and its derived classes cause they generate internal state automatically
     *
     * @author Julius
     * @date 20.10.2021
     */
    class AssetSeeder {
    public:
        using value_type = AssetSeeder;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 20.10.2021
         */
        AssetSeeder();

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.10.2021
         */
        ~AssetSeeder() noexcept;

    private:
        std::deque<ptr<ember::Asset>(*)(void)> _constructors;

    public:
        bool autoRegister(ptr<ember::Asset> (*fncPtr_)(void));

        void seed(ptr<void> scheduler_);
    };
}
