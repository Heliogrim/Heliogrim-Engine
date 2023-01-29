#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "CacheRequest.hpp"
#include "CacheResponse.hpp"
#include "LoaderStageTraits.hpp"
#include "__fwd.hpp"
#include "../Manage/ResourceConcept.hpp"

namespace ember::engine::resource::loader {
    class LoaderBase;

    template <class Type_>
    concept IsLoader = _STD derived_from<Type_, LoaderBase>;

    class __declspec(novtable) LoaderBase :
        public EmberObject {
    public:
        template <IsRequestValueType, IsResponseValueType>
        friend class Loader;

    public:
        using this_type = LoaderBase;

    private:
        LoaderBase() noexcept = default;

    public:
        virtual ~LoaderBase() noexcept = default;

    public:
        [[nodiscard]] virtual smr<ResourceBase> operator()(
            const ptr<const assets::Asset> asset_,
            const ptr<const void> options_
        ) = 0;
    };

    template <IsRequestValueType RequestType_, IsResponseValueType ResponseType_>
    class __declspec(novtable) Loader :
        public LoaderBase {
    public:
        template <
            IsRequestValueType,
            IsResponseValueType,
            typename,
            class,
            class,
            class,
            class
        >
        friend class LoaderChain;

    public:
        using stage_traits = LoaderStageTraits<CacheRequest<RequestType_>, CacheResponse<ResponseType_>>;

    private:
        Loader() noexcept = default;

    public:
        ~Loader() noexcept override = default;

    public:
        [[nodiscard]] virtual typename stage_traits::response_value_type operator()(
            mref<typename stage_traits::request_value_type> request_,
            mref<typename stage_traits::request_options_type> options_
        ) = 0;

        [[nodiscard]] virtual typename stage_traits::response_value_type operator()(
            mref<typename stage_traits::request_value_type> request_,
            mref<typename stage_traits::request_options_type> options_,
            mref<typename stage_traits::stream_options_type> streamOptions_
        ) = 0;

    public:
        [[nodiscard]] smr<ResourceBase> operator()(
            const ptr<const assets::Asset> asset_,
            const ptr<const void> options_
        ) override {
            static_assert(_STD is_convertible_v<decltype(asset_), typename stage_traits::request_value_type>);
            static_assert(_STD is_convertible_v<decltype(options_), ptr<typename stage_traits::request_options_type>>);

            if constexpr (_STD derived_from<
                typename stage_traits::response_value_type,
                smr<typename stage_traits::response_type::value_type>
            >) {
                return this->operator()(
                    asset_,
                    *static_cast<const ptr<const typename stage_traits::request_options_type>>(options_)
                );
            }

            return make_smr(
                this->operator()(
                    asset_,
                    *static_cast<const ptr<const typename stage_traits::request_options_type>>(options_)
                )
            );
        }
    };
}
