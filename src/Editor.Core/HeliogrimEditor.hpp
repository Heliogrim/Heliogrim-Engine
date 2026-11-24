#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Heliogrim;
#endif

namespace hg {
    class HeliogrimEditor :
        public Heliogrim {
    public:
        [[nodiscard]] static Session getEditorSession() noexcept;

        [[nodiscard]] static Session getSession();

        [[nodiscard]] static Session getSession(std::nothrow_t) noexcept;
    };
}
