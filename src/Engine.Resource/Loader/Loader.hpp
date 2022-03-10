#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "../Manage/Resource.hpp"

namespace ember::engine::res {

    template <assets::IsAsset Type_>
    struct LoaderOptions;

    class __declspec(novtable) LoaderBase {
    public:
        template <assets::IsAsset AssetType_>
        friend class Loader;

    public:
        using value_type = LoaderBase;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        template <typename Type_>
        using result_wrapper_type = _STD type_identity_t<Type_>;
        using result_data_type = ptr<Resource>;
        using result_type = result_wrapper_type<result_data_type>;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        LoaderBase() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 10.09.2021
         */
        virtual ~LoaderBase() noexcept = default;

    public:
        /**
         * Function call operator
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  asset_ The asset to load.
         * @param  options_ (Optional) Options for controlling the operation.
         *
         * @returns The result of the operation.
         */
        [[nodiscard]] virtual result_type operator()(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr) = 0;
    };

    template <assets::IsAsset AssetType_>
    class __declspec(novtable) Loader :
        public LoaderBase {
    public:
        using base_type = LoaderBase;

        using value_type = Loader<AssetType_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using result_type = base_type::result_wrapper_type<ptr<Resource>>;
        using options_type = ptr<LoaderOptions<AssetType_>>;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        Loader() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.08.2021
         */
        ~Loader() override = default;

    protected:
        [[nodiscard]] result_type operator(
        )(const ptr<assets::Asset> asset_, ptr<void> options_ = nullptr) final override {
            return this->operator()(static_cast<const ptr<AssetType_>>(asset_), static_cast<options_type>(options_));
        }

    public:
        [[nodiscard]] virtual result_type operator()(const ptr<AssetType_> asset_, options_type options_ = nullptr) = 0;

    };
}
