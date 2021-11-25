#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "EmberClass.hpp"

namespace ember {

    class EmberObject {
        using this_type = EmberObject;

    public:
        [[nodiscard]] ptr<EmberClass> getClass() const noexcept;
    };

}
