#pragma once

#include <ostream>

#include "../Types.hpp"
#include "../Wrapper.hpp"
#include "../String.hpp"

#include "../Concurrent/SpinLock.hpp"

namespace hg::profiling {
    struct ProfileResult {
        _STD string name;
        u64 start;
        u64 end;
        u64 thread;
    };

    class Profiler {
    public:
        using value_type = Profiler;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Profiler() noexcept;

    public:
        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Profiler(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        Profiler(mref<value_type>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.09.2021
         */
        ~Profiler() noexcept;

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
        reference_type operator=(mref<value_type>) noexcept = delete;

    private:
        /** The singleton instance */
        static ptr<Profiler> _instance;

    public:
        /**
         * Gets a reference to the internal stored singleton instance
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A reference_type.
         */
        static const ptr<Profiler> get();

        /**
         * Gets a pointer to the internal stored singleton instance
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A const ptr&lt;value_type&gt;
         */
        static const ptr<value_type> get(_STD nothrow_t) noexcept;

        /**
         * Gets a reference to the internal stored singleton instance or created one
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A reference_type.
         */
        static reference_type make();

        /**
         * Gets a reference to the internal stored singleton instance or created one
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @returns A reference_type.
         */
        static const ptr<value_type> make(_STD nothrow_t) noexcept;

        /**
         * Destroys the internal stored singleton instance
         *
         * @author Julius
         * @date 21.09.2021
         */
        static void destroy();

    private:
        _STD string _name;
        ptr<_STD ostream> _output;
        u64 _count;

    private:
        concurrent::SpinMutex _mtx;

    public:
        /**
         * Flushes this
         *
         * @author Julius
         * @date 21.09.2021
         */
        void flush();

        /**
         * Starts a session
         *
         * @author Julius
         * @date 21.09.2021
         *
         * @param  name_ The name.
         * @param  file_ (Optional) The file.
         */
        void startSession(cref<string_view> name_, cref<string_view> file_ = "result.json"sv);

        /**
         * Stops a session
         *
         * @author Julius
         * @date 21.09.2021
         */
        void stopSession();

    private:
        void writeProfile(cref<ProfileResult> result_);

        void writeFooter();

        void writeHeader();

    private:
        friend ref<Profiler> operator<<(ref<Profiler>, cref<ProfileResult>);
    };

    /**
     * Write ProfileResult to Profiler
     *
     * @author Julius
     * @date 21.09.2021
     *
     * @param [in,out] self_ The self.
     * @param          result_ The result.
     *
     * @returns The shifted result.
     */
    ref<Profiler> operator<<(ref<Profiler> self_, cref<ProfileResult> result_);
}
