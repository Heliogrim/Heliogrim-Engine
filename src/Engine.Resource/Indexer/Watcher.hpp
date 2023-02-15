#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Wrapper.hpp>

#ifdef ENV_MSVC
#include <Engine.Common/__macro.hpp>
#include <Windows.h>
#include <Fileapi.h>
#endif

#include <functional>
#include <unordered_map>

#include "../File.hpp"

namespace ember::engine::res {
    class Watcher {
    public:
        using value_type = Watcher;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        Watcher(cref<fs::File> root_);

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        Watcher(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 23.09.2021
         */
        Watcher(mref<value_type>) = delete;

        ~Watcher();

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 23.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) = delete;

    public:
        void tidy();

    private:
        void setup();

        void notify(const bool publish_ = true);

    private:
        fs::File _root;

    public:
        cref<fs::File> root() const noexcept;

        fs::File root() noexcept;

    public:
        [[nodiscard]] operator const fs::File() const noexcept;

        [[nodiscard]] operator fs::File() noexcept;

    private:
        HANDLE _handle;
        HANDLE _waitHandle;

    private:
        _STD unordered_map<fs::File, _STD filesystem::file_time_type> _state;

        _STD function<void(fs::File)> _createCallback;
        _STD function<void(fs::File)> _modifiedCallback;
        _STD function<void(fs::File)> _eraseCallback;

    public:
        void setCreateCallback(cref<std::function<void(fs::File file_)>> callback_) noexcept;

        void setModifiedCallback(cref<std::function<void(fs::File file_)>> callback_) noexcept;

        void setEraseCallback(cref<std::function<void(fs::File file_)>> callback_) noexcept;
    };
}
