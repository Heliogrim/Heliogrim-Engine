#pragma once
#include "stdafx.h"

namespace hg::__internal::types {
    template <typename Type, typename Traits>
    class basic_istream {
    public:
        using int_type = typename Traits::int_type;
        using pos_type = typename Traits::pos_type;
        using off_type = typename Traits::off_type;

        /**
         * \brief 
         * \param stream_ 
         */
        basic_istream(const std::basic_istream<Type, Traits>&& stream_) :
            _stream(static_cast<std::basic_istream<Type, Traits>*>(malloc(sizeof(stream_)))) {
            memcpy(_stream, &(std::move(stream_)), sizeof(stream_));
        }

        /**
         * \brief 
         * \param buf_ 
         * \param size_ 
         */
        constexpr void get(char* buf_, size_t size_) {
            _stream->get(buf_, size_);
        }

        /**
         * \brief 
         * \param pos_ 
         */
        constexpr void seek(pos_type pos_) {
            _stream->seekg(pos_);
        }

        /**
         * \brief 
         * \param off_ 
         * \param dir_ 
         */
        constexpr void seek(off_type off_, std::ios_base::seekdir dir_) {
            _stream->seekg(off_, dir_);
        }

        /**
         * \brief 
         * \tparam _Ty 
         * \param val_ 
         * \return 
         */
        template <typename _Ty>
        constexpr basic_istream<Type, Traits> operator>>(_Ty& val_) {
            (*_stream) >> val_;
            return *this;
        }

        /**
         * \brief Copy Assignment is deleted cause of critical stream handling
         * \param other_ 
         * \return 
         */
        basic_istream<Type, Traits>& operator=(const basic_istream<Type, Traits>& other_) = delete;

        /**
         * \brief Move Assignment will cause only one stream object to own internal stream object
         * \param other_ 
         * \return 
         */
        basic_istream<Type, Traits>& operator=(basic_istream<Type, Traits>&& other_) noexcept {
            if (this != &other_) {
                _stream = std::exchange(other_._stream, nullptr);
            }
            return *this;
        }

    private:
        std::basic_istream<Type, Traits>* _stream = nullptr;
    };

    template <typename Type, typename Traits>
    class basic_ostream {
    public:
        using int_type = typename Traits::int_type;
        using pos_type = typename Traits::pos_type;
        using off_type = typename Traits::off_type;

        /**
         * \brief 
         * \param stream_ 
         */
        basic_ostream(const std::basic_ostream<Type, Traits>&& stream_) :
            _stream(static_cast<std::basic_ostream<Type, Traits>*>(malloc(sizeof(stream_)))) {
            memcpy(_stream, &(std::move(stream_)), sizeof(stream_));
        }

        /**
         * \brief 
         * \tparam _Ty 
         * \param val_ 
         * \return 
         */
        template <typename _Ty>
        constexpr basic_ostream<Type, Traits> operator<<(_Ty& val_) {
            (*_stream) << val_;
            return *this;
        }

        /**
         * \brief 
         * \param other_ 
         * \return 
         */
        basic_ostream<Type, Traits>& operator=(const basic_ostream<Type, Traits>& other_) = delete;

        /**
         * \brief 
         * \param other_ 
         * \return 
         */
        basic_ostream<Type, Traits>& operator=(basic_ostream<Type, Traits>&& other_) noexcept {
            if (this != &other_) {
                _stream = std::exchange(other_._stream, nullptr);
            }
            return *this;
        }

    private:
        std::basic_ostream<Type, Traits>* _stream = nullptr;
    };

    template <typename Type, typename Traits>
    class basic_iostream {
    public:
        using int_type = typename Traits::int_type;
        using pos_type = typename Traits::pos_type;
        using off_type = typename Traits::off_type;

        /**
         * \brief 
         * \tparam _Ty 
         * \param val_ 
         * \return 
         */
        template <typename _Ty>
        constexpr basic_iostream<Type, Traits> operator>>(_Ty& val_) {
            (*_stream) >> val_;
            return *this;
        }

        /**
         * \brief 
         * 
         * \tparam _Ty 
         * \param val_ 
         * \return 
         */
        template <typename _Ty>
        constexpr basic_iostream<Type, Traits> operator<<(_Ty& val_) {
            (*_stream) << val_;
            return *this;
        }

        /**
         * \brief 
         * \param pos_ 
         */
        constexpr void seek(pos_type pos_) {
            _stream->seekg(pos_);
        }

        /**
         * \brief 
         * \param off_ 
         * \param dir_ 
         */
        constexpr void seek(off_type off_, std::ios_base::seekdir dir_) {
            _stream->seekg(off_, dir_);
        }

        /**
         * \brief 
         * \param other_ 
         * \return 
         */
        basic_iostream<Type, Traits>& operator=(const basic_iostream<Type, Traits>& other_) = delete;

        /**
         * \brief 
         * \param other_ 
         * \return 
         */
        basic_iostream<Type, Traits>& operator=(basic_iostream<Type, Traits>&& other_) noexcept {
            if (this != &other_) {
                _stream = std::exchange(other_._stream, nullptr);
            }
            return *this;
        }

    private:
        std::basic_iostream<Type, Traits>* _stream = nullptr;
    };

    using istream = basic_istream<char, std::char_traits<char>>;
    using ostream = basic_ostream<char, std::char_traits<char>>;
    using iostream = basic_iostream<char, std::char_traits<char>>;
}
