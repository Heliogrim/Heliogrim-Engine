#pragma once

#include <Ember/Ember.hpp>

namespace ember {
    class EmberEditor :
        public Ember {
    public:
        [[nodiscard]] static Session getEditorSession() noexcept;

        [[nodiscard]] static Session getSession();

        [[nodiscard]] static Session getSession(std::nothrow_t) noexcept;
    };
}
