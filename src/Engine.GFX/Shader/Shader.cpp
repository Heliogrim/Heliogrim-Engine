#include "Shader.hpp"

using namespace hg::engine::gfx::shader;
using namespace hg::engine::gfx;
using namespace hg;

vk::ShaderStageFlagBits vkTranslate(const ShaderType type_) {
    switch (type_) {
        case ShaderType::eAll: {
            return vk::ShaderStageFlagBits::eAll;
        }
        case ShaderType::eVertex: {
            return vk::ShaderStageFlagBits::eVertex;
        }
        case ShaderType::eFragment: {
            return vk::ShaderStageFlagBits::eFragment;
        }
        case ShaderType::eCompute: {
            return vk::ShaderStageFlagBits::eCompute;
        }
        case ShaderType::eGeometry: {
            return vk::ShaderStageFlagBits::eGeometry;
        }
        /*
        case eTessellation: {
            return vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eTessellationEvaluation;
        }
         */
        default: {
            throw _STD runtime_error("Can not translate undefined `ShaderType` to `vk::ShaderStageFlagBits`.");
        }
    }
}

Shader::Shader(
    const ShaderType type_,
    const string& name_,
    const Vector<ShaderBinding>& bindings_,
    vk::ShaderModule vkShader_
) :
    _type(type_),
    _name(name_),
    _bindings(bindings_),
    _vkShader(vkShader_) {}

Shader::operator bool() const noexcept {
    return (_type != ShaderType::eNone && _type != ShaderType::eAll) && _vkShader;
}

ShaderType Shader::type() const noexcept {
    return _type;
}

const string& Shader::name() const noexcept {
    return _name;
}

const std::vector<ShaderBinding>& Shader::bindings() const noexcept {
    return _bindings;
}

vk::PipelineShaderStageCreateInfo Shader::vkShaderStageInfo() const noexcept {
    return vk::PipelineShaderStageCreateInfo {
        vk::PipelineShaderStageCreateFlags(),
        vkTranslate(_type),
        _vkShader,
        /*_name.data(),*/
        "main",
        nullptr
    };
}
