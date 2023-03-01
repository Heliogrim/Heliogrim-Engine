#pragma once

#include "Inc.hpp"

namespace hg {
    template <typename Class>
    class Singleton {
    public:
        static Class& ref() {
            return *_instance;
        }

        static Class* ptr() {
            return _instance;
        }

    private:
        static Class* _instance = nullptr;

    protected:
        Singleton() { }
    };
}
