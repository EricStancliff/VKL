#include <Shader.h>

#include <Device.h>

#include <shaderc/shaderc.h>
#include <fstream>
#include <iostream>
namespace vkl
{
    shaderc_shader_kind shaderKind(VkShaderStageFlagBits stage)
    {
        switch (stage)
        {
        case VK_SHADER_STAGE_VERTEX_BIT:
        {
            return shaderc_vertex_shader;
                break;
        }
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        {
            return shaderc_tess_control_shader;
            break;
        }
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        {
            return shaderc_tess_evaluation_shader;
            break;
        }
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        {
            return shaderc_geometry_shader;
            break;
        }
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        {
            return shaderc_fragment_shader;
            break;
        }
        case VK_SHADER_STAGE_COMPUTE_BIT:
        {
            return shaderc_compute_shader;
            break;
        }
        default:
        {
            return shaderc_glsl_infer_from_source;
            break;
        }
        }
        return shaderc_glsl_infer_from_source;
    }


	ShaderModule::ShaderModule(const Device& device, std::shared_ptr<const ShaderData> shader, VkShaderStageFlagBits shaderStage)
	{
		m_shaderData = shader;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader->dataSize();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shader->data());
        if (vkCreateShaderModule(device.handle(), &createInfo, nullptr, &_shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Error");
        }

	}
    VkShaderModule ShaderModule::handle() const
    {
        return _shaderModule;
    }

    /******************************************************************************************/

    GLSLShader::GLSLShader(const char* shader, VkShaderStageFlagBits stage)
    {
        _stage = stage;
        init(shader, strlen(shader));
    }
    GLSLShader::GLSLShader(const std::filesystem::path& path, VkShaderStageFlagBits stage)
    {
        _stage = stage;
        _fileName = path.string();

        std::ifstream file(_fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Error");
            return;
        }

        _size = (size_t)file.tellg();
        _ownedBuffer.resize(_size);

        file.seekg(0);
        file.read(_ownedBuffer.data(), _size);

        file.close();

        _data = _ownedBuffer.data();
        init(_data, _size);
    }
    GLSLShader::~GLSLShader()
    {
        shaderc_result_release((shaderc_compilation_result_t)_result);
    }
    const char* const GLSLShader::data() const
    {
        return _data;
    }
    size_t GLSLShader::dataSize() const
    {
        return _size;
    }

    std::string GLSLShader::fileName() const
    {
        return _fileName;
    }
    VkShaderStageFlagBits GLSLShader::stage() const
    {
        return _stage;
    }
    void GLSLShader::init(const char* data, size_t size)
    {
        auto compiler = shaderc_compiler_initialize();
        auto options = shaderc_compile_options_initialize();

        auto result = shaderc_compile_into_spv(compiler, data, size, shaderKind(_stage), _fileName.c_str(), "main", options);

        if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
            throw std::runtime_error("Error");
            return;
        }

        _data = shaderc_result_get_bytes(result);
        _size = shaderc_result_get_length(result);

        _result = (void*)result;

        shaderc_compiler_release(compiler);
        shaderc_compile_options_release(options);
    }


    /******************************************************************************************/


    SPVShader::SPVShader(const char* shader, VkShaderStageFlagBits stage)
    {
        _stage = stage;
        _data = shader;
        _size = strlen(shader);
    }
    SPVShader::SPVShader(const std::filesystem::path& path, VkShaderStageFlagBits stage)
    {
        _stage = stage;
        _fileName = path.string();

        std::ifstream file(_fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Error");
            return;
        }

        _size = (size_t)file.tellg();
        _ownedBuffer.resize(_size);

        file.seekg(0);
        file.read(_ownedBuffer.data(), _size);

        file.close();

        _data = _ownedBuffer.data();
    }
    const char* const SPVShader::data() const
    {
        return _data;
    }
    size_t SPVShader::dataSize() const
    {
        return _size;
    }

    std::string SPVShader::fileName() const
    {
        return _fileName;
    }
    VkShaderStageFlagBits SPVShader::stage() const
    {
        return _stage;
    }
}