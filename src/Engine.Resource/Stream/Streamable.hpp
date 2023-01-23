#pragma once

#include "Resource.hpp"
#include "../Loader/Loader.hpp"

namespace ember::engine::res::partial {

    template <IsResource Type_>
    class __declspec(novtable) Streamable :
        public Type_ {
    public:
        using this_type = Streamable<Type_>;
        using underlying_type = Type_;

    protected:
        // TODO: Check whether we can type enforce a stream loader
        wptr<LoaderBase> _streamLoader;

    public:
        [[nodiscard]] bool hasStreamLoader() const noexcept {
            return not _streamLoader.expired();
        }

        [[nodiscard]] cref<wptr<LoaderBase>> streamLoader() const noexcept {
            return _streamLoader;
        }

        void setStreamLoader(mref<wptr<LoaderBase>> loader_) noexcept {
            _streamLoader = _STD move(loader_);
        }

    public:
        virtual void streamLoad(const ptr<EmberObject> options_) = 0;

        virtual void streamUnload(const ptr<EmberObject> options_) = 0;
    };
}
