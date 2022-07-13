#pragma once

#include <sstream>

#include "Types.hpp"
#include "String.hpp"
#include "Wrapper.hpp"

namespace ember {

    class Url {
    public:
        using this_type = Url;

    public:
        Url(cref<string_view> scheme_, cref<string_view> path_) :
            _scheme(scheme_),
            _user(),
            _password(),
            _host(),
            _port(0ui16),
            _path(path_),
            _query(),
            _fragment() {}

        Url(cref<this_type> other_) = default;

        Url(mref<this_type> other_) = default;

        ~Url() = default;

    private:
        string _scheme;

    public:
        [[nodiscard]] bool hasScheme() const noexcept {
            return !_scheme.empty();
        }

        [[nodiscard]] string_view scheme() const noexcept {
            return _scheme;
        }

    private:
        string _user;

    public:
        [[nodiscard]] bool hasUser() const noexcept {
            return !_user.empty();
        }

        [[nodiscard]] string_view user() const noexcept {
            return _user;
        }

    private:
        string _password;

    public:
        [[nodiscard]] bool hasPassword() const noexcept {
            return !_password.empty();
        }

        [[nodiscard]] string_view password() const noexcept {
            return _password;
        }

    private:
        string _host;

    public:
        [[nodiscard]] bool hasHost() const noexcept {
            return !_host.empty();
        }

        [[nodiscard]] string_view host() const noexcept {
            return _host;
        }

    private:
        u16 _port;

    public:
        [[nodiscard]] bool hasPort() const noexcept {
            return _port;
        }

        [[nodiscard]] u16 port() const noexcept {
            return _port;
        }

    private:
        string _path;

    public:
        [[nodiscard]] bool hasPath() const noexcept {
            return !_path.empty();
        }

        [[nodiscard]] string_view path() const noexcept {
            return _path;
        }

    private:
        string _query;

    public:
        [[nodiscard]] bool hasQuery() const noexcept {
            return !_query.empty();
        }

        [[nodiscard]] string_view query() const noexcept {
            return _query;
        }

    private:
        string _fragment;

    public:
        [[nodiscard]] bool hasFragment() const noexcept {
            return !_fragment.empty();
        }

        [[nodiscard]] string_view fragment() const noexcept {
            return _fragment;
        }

    public:
        /**
         * Check whether Url is empty
         *
         * @author Julius
         * @date 27.06.2022
         *
         * @returns True is this url is empty, otherwise false.
         */
        [[nodiscard]] bool empty() const noexcept {
            return _scheme.empty() && _user.empty() && _password.empty() && _host.empty() && _port == 0ui16 && _path.
                empty() && _query.empty() && _fragment.empty();
        }

    public:
        /**
         *
         * @details `<scheme>://<user><:?><password><@?><host><:?><port><path><?:><query><#?><fragment>`
         */
        [[nodiscard]] string_view encode() const noexcept {

            _STD stringstream ss {};
            ss << _scheme << "://"sv;
            // TODO: user, password, host, port
            ss << _path;

            return ss.view();
        }

        /**
         * Cast this to a string_view
         *
         * @author Julius
         * @date 08.12.2021
         *
         * @returns A string_view encoding this url.
         */
        [[nodiscard]] operator string_view() const noexcept {
            return encode();
        }
    };
}
