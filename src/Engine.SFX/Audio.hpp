﻿#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/CoreModule.hpp>

namespace ember::engine {
    class Audio final :
        public core::CoreModule {
    public:
        using value_type = Audio;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        Audio(const non_owning_rptr<Engine> engine_) noexcept;

        ~Audio() noexcept override;

    public:
        void setup();

        void schedule();

        void desync() override;

        void destroy() override;

    private:
        /**
         * Sets up the importer
         *
         * @author Julius
         * @date 12.09.2021
         */
        void setupImporter();
    };
}
