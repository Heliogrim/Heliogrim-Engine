#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "LoaderTraits.hpp"
#include "__fwd.hpp"
#include "../Manage/ResourceConcept.hpp"

#include "../Manage/UniqueResource.hpp"

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
            ptr<assets::Asset> asset_,
            ptr<void> options_
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
        using traits = LoaderTraits<RequestType_, ResponseType_>;

    private:
        Loader() noexcept = default;

    public:
        ~Loader() noexcept override = default;

    public:
        [[nodiscard]] virtual typename traits::response::type operator()(
            mref<typename traits::request::type> request_,
            mref<typename traits::request::options> options_
        ) = 0;

        [[nodiscard]] virtual typename traits::response::type operator()(
            mref<typename traits::request::type> request_,
            mref<typename traits::request::options> options_,
            mref<typename traits::request::stream> streamOptions_
        ) = 0;

    public:
        [[nodiscard]] smr<ResourceBase> operator()(
            ptr<assets::Asset> asset_,
            ptr<void> options_
        ) override {

            using request_type = typename traits::request::type;
            using options_type = typename traits::request::options;

            static_assert(_STD is_convertible_v<decltype(asset_), request_type>);
            static_assert(_STD is_convertible_v<decltype(options_), ptr<options_type>>);

            if constexpr (_STD derived_from<
                typename traits::response::type,
                smr<typename traits::response::value_type>
            >) {
                return this->operator()(
                    _STD move(static_cast<request_type>(asset_)),
                    _STD move(*static_cast<ptr<options_type>>(options_))
                );
            }

            /*
            return make_smr(
                this->operator()(
                    _STD move(static_cast<request_type>(asset_)),
                    _STD move(*static_cast<ptr<options_type>>(options_))
                )
            );
             */
            return {};
        }
    };
}
