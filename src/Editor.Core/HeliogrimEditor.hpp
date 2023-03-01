#pragma once

#include <Heliogrim/Heliogrim.hpp>

namespace hg {
    class HeliogrimEditor :
        public Heliogrim {
    public:
        [[nodiscard]] static Session getEditorSession() noexcept;

        [[nodiscard]] static Session getSession();

        [[nodiscard]] static Session getSession(std::nothrow_t) noexcept;
    };
}
