#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <Engine.Common/String.hpp>

namespace hg::engine::gfx::render {
    inline static string read_shader_file(string name_) {

        const auto root = R"(R:\Development\C++\Vulkan API\Game\resources\shader\)";
        std::filesystem::path file { root };
        file.append(name_);

        if (not exists(file)) {
            __debugbreak();
            return {};
        }

        auto ifs = _STD ifstream(file, std::ios_base::in | std::ios_base::binary);

        ifs.seekg(0, _STD ios::end);
        const auto fsize = ifs.tellg();
        ifs.seekg(0, _STD ios::beg);

        string tmp {};
        tmp.resize(fsize);

        ifs.read(tmp.data(), fsize);
        assert(!ifs.bad());

        ifs.close();
        return tmp;
    }
}
