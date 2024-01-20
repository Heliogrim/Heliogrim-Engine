#pragma once

#include <exception>
#include <iostream>
#include <iomanip>
#include <Engine.Common/Exception/__macro.hpp>
#include <Engine.Common/Collection/List.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "tmp.hpp"

namespace hg::engine::res::types {
    class StreamUtils {
    public:
        /**
         * Writes
         *
         * @tparam T Generic type parameter.
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         *
         * ### tparam T Generic type parameter.
         */
        template <typename T>
        static void write(const T& src_, std::ostream& dst_) {
            dst_.write(reinterpret_cast<const char*>(&src_), sizeof(T));
        }

        /**
         * Reads the given source
         *
         * @tparam T Generic type parameter.
         * @param [in,out] src_ Source for the.
         *
         * @return A T.
         *
         * ### tparam T Generic type parameter.
         */
        template <typename T>
        static T read(std::istream& src_) {
            T dst;
            src_.read(reinterpret_cast<char*>(&dst), sizeof(T));
            return dst;
        }

        /**
         * Writes a n
         *
         * @tparam T Generic type parameter.
         * @param 		   src_  Source for the.
         * @param 		   size_ The size.
         * @param [in,out] dst_  Destination for the.
         */
        template <typename T>
        static void writeN(const T* src_, const uint32_t size_, std::ostream& dst_) {
            EXCEPTION_UNSUPPORTED_METHOD
        }

        /**
         * Reads a n
         *
         * @tparam T Generic type parameter.
         * @param [in,out] src_  Source for the.
         * @param 		   size_ (Optional) The size.
         *
         * @return The n.
         *
         * ### tparam T Generic type parameter.
         */
        template <typename T>
        static std::vector<T> readN(std::istream& src_, const uint32_t* size_ = nullptr) {
            uint32_t length = (size_ != nullptr) ? (*size_) : StreamUtils::read<uint32_t>(src_);

            std::vector<T> dst = std::vector<T>(0);
            dst.resize(length);

            for (uint32_t i = 0; i < length; i++)
                dst[i] = StreamUtils::read<T>(src_);

            return dst;
        }

        /* Generic Type = std::string */

        /**
        * Writes
        *
        * @param 		   src_ Source for the.
        * @param [in,out] dst_ Destination for the.
        */
        template <>
        static void write(const std::string& src_, std::ostream& dst_) {
            const uint32_t length = UINT32_T(src_.length());
            const char* seq = src_.c_str();

            write<uint32_t>(length, dst_);
            dst_ << seq;
        }

        /**
        * Reads the given source
        *
        * @param [in,out] src_ Source for the.
        *
        * @return A std::string.
        */
        template <>
        static std::string read(std::istream& src_) {
            const uint32_t length = read<uint32_t>(src_);

            char* seq = new char[length];
            src_.read(seq, length);
            std::string dst = std::string(seq, length);
            delete[] seq;

            return dst;
        }

        /**
         * Writes a n
         *
         * @param 		   src_   Source for the.
         * @param 		   count_ Number of.
         * @param [in,out] dst_   Destination for the.
         */
        template <>
        static void writeN(const std::string* src_, const uint32_t count_, std::ostream& dst_) {
            write<uint32_t>(count_, dst_);

            for (uint32_t i = 0; i < count_; i++)
                write<std::string>(src_[i], dst_);
        }

        /* Generic Type = vec3 */

        /**
         * Writes
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         */
        template <>
        static void write(const hg::math::vec3& src_, std::ostream& dst_) {
            write<float>(src_.x, dst_);
            write<float>(src_.y, dst_);
            write<float>(src_.z, dst_);
        }

        /**
         * Reads the given source
         *
         * @param [in,out] src_ Source for the.
         *
         * @return A vec3.
         */
        template <>
        static hg::math::vec3 read(std::istream& src_) {
            hg::math::vec3 dst {};

            dst.x = read<float>(src_);
            dst.y = read<float>(src_);
            dst.z = read<float>(src_);

            return dst;
        }

        /* Generic Type = charArray */

        /* !! Attention !! char array does not consume a size and don't write a size */
        /* it is not possible to read char array with precision */
        /* alternative to char array use std::string as generic type to get char array with "dynamic" length which is possible to write and read */
        /* write does not support undefined length - use write_n instead */

        /**
         * Writes
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         */
        template <>
        static void write(const charArray& src_, std::ostream& dst_) {
            EXCEPTION_UNSUPPORTED_METHOD
        }

        /**
         * Reads the given source
         *
         * @param [in,out] src_ Source for the.
         *
         * @return A charArray.
         */
        template <>
        static charArray read(std::istream& src_) {
            EXCEPTION_UNSUPPORTED_METHOD
        }

        /**
         * Writes src_ with length n
         *
         * @tparam charArray Type of the character array.
         * @param 		   src_  Source for the.
         * @param length_
         * @param 		   size_ The size.
         * @param [in,out] dst_  Destination for the.
         */
        template <>
        static void writeN<char>(const char* src_, const uint32_t length_, std::ostream& dst_) {
            dst_.write(src_, sizeof(char) * length_);
        }
    };
}
