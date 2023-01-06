#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine {
    class Engine;
}

namespace ember::engine::core {
    class __declspec(novtable) CoreModule {
    public:
        CoreModule(const non_owning_rptr<Engine> engine_) :
            _engine(engine_) {}

        virtual ~CoreModule() = default;

    protected:
        const non_owning_rptr<Engine> _engine;

    public:
        virtual void setup() = 0;

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void destroy() = 0;
    };
}
