#pragma once

#include <atomic>
#include <span>
#include <Engine.Common/Wrapper.hpp>

#include "ModuleSource.hpp"
#include "SpirvByteCode.hpp"
#include "Spirv/SpirvDialect.hpp"
#include "Spirv/SpirvSrcLang.hpp"
#include "Spirv/SpirvTargetClient.hpp"
#include "Spirv/SpirvTargetVersion.hpp"

namespace hg::engine::accel {
    extern std::atomic_uint_fast32_t extern_compiler_ref_count;

    class SpirvCompiler {
    public:
        using this_type = SpirvCompiler;

    public:
        SpirvCompiler();

        ~SpirvCompiler();

    public:
        SpirvSrcLang _srcLang;
        SpirvDialect _dialect;
        SpirvTargetClient _targetClient;
        SpirvTargetVersion _targetVersion;

    public:
        [[nodiscard]] SpirvByteCode compile(
            cref<ModuleSource> module_,
            cref<InlineAutoArray<lang::TextBlock>> source_
        ) const;
    };
}
