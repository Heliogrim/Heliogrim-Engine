#pragma once

#include <Engine.Common/String.hpp>

// TODO: Move to Common Library
namespace ember {
    struct Url {
        ember::string unwrap;

        /**
         * Cast that converts the given to a string
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns The result of the operation.
         */
        [[nodiscard]] operator ember::string() const noexcept {
            return unwrap;
        }
    };
}
