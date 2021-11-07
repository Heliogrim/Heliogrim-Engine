#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/Registry.hpp>
#include <Engine.Session/Session.hpp>

#include "Staged/StagedComponentProcessor.hpp"

namespace ember::engine::ecs {

    class Subsystem {
    public:
        using value_type = Subsystem;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        Subsystem(cref<sptr<Session>> session_, ptr<System> system_) noexcept;

        ~Subsystem() noexcept;

    private:
        void setupComponents();

    public:
        void setup();

        void schedule();

    private:
        sptr<Session> _session;

    public:
        [[nodiscard]] sptr<Session> session() const noexcept;

    private:
        ptr<System> _system;

    public:
        [[nodiscard]] ptr<System> system() const noexcept;

    private:
        subsystem::StagedComponentProcessor _stagedProcessor;
    };

}
