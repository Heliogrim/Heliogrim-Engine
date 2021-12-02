#include "ActorInitializer.hpp"

using namespace ember;

ref<ActorInitializer> ActorInitializer::get() noexcept {
    static ActorInitializer instance {};
    return instance;
}
