#pragma once

#include <chrono>

#include "../String.hpp"
#include "../Wrapper.hpp"

namespace ember::profiling {

    class Stopwatch {
    public:
        using value_type = Stopwatch;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Stopwatch() noexcept;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @param  name_ The name.
         * @param  stopped_ True to stopped.
         */
        Stopwatch(const char* name_, const bool stopped_);

        /**
         * Constructor
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @param  name_ The name.
         * @param  stopped_ True to stopped.
         */
        Stopwatch(cref<string_view> name_, const bool stopped_);

    public:
        /**
         * Copy constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Stopwatch(const_reference_type) = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Stopwatch(mref<Stopwatch>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        ~Stopwatch() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<Stopwatch>) noexcept = delete;

    protected:
        string _name;
        bool _stopped;
        _STD chrono::high_resolution_clock::time_point _timestamp;

    public:
        /**
         * Starts this 
         *
         * @author Julius
         * @date 21.09.2021
         */
        void start() noexcept;

        /**
         * Stops this
         *
         * @author Julius
         * @date 21.09.2021
         */
        void stop() noexcept;

    public:
        /**
         * Gets the name of this stopwatch
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A const string_view
         */
        const string_view name() const noexcept;

        /**
         * Gets a reference to the name
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A reference
         */
        ref<string> name() noexcept;
    };

    class ScopedStopwatch :
        public Stopwatch {
    public:
        using base_type = Stopwatch;

        using value_type = ScopedStopwatch;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @param  name_ The name.
         */
        ScopedStopwatch(const char* name_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        ScopedStopwatch(mref<value_type>) noexcept = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        ScopedStopwatch(const_reference_type) = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        ~ScopedStopwatch() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move assignment operator
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) noexcept = delete;
    };
}
