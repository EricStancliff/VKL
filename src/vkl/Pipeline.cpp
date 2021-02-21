#include <Pipeline.h>

#include <Shader.h>

#include <shared_mutex>

namespace vkl
{
	class ShaderCache
	{
	public:

		std::shared_ptr<const ShaderData> getOrCreateGLSL(const char* shader, VkShaderStageFlagBits stage)
		{
			{
				std::shared_lock lock(_mutex);
				auto findShader = std::find_if(_shaders.begin(), _shaders.end(), [&](const std::shared_ptr<const ShaderData>& shaderPtr)
					{
						return shaderPtr->data() == shader && shaderPtr->stage() == stage;
					});
				if (findShader != _shaders.end())
				{
					return *findShader;
				}
			}

			std::unique_lock lock(_mutex);

			std::shared_ptr<ShaderData> newShader = std::make_shared<GLSLShader>(shader, stage);
			_shaders.push_back(newShader);
			return newShader;

		}
		std::shared_ptr<const ShaderData> getOrCreateGLSL(const std::filesystem::path& path, VkShaderStageFlagBits stage)
		{
			std::error_code ec;
			auto absolute = std::filesystem::absolute(path, ec);
			if (!std::filesystem::exists(absolute, ec))
				return nullptr;

			{
				std::shared_lock lock(_mutex);
				auto findShader = std::find_if(_shaders.begin(), _shaders.end(), [&](const std::shared_ptr<const ShaderData>& shaderPtr)
					{
						return shaderPtr->fileName() == path && shaderPtr->stage() == stage;
					});
				if (findShader != _shaders.end())
				{
					return *findShader;
				}
			}

			std::unique_lock lock(_mutex);

			std::shared_ptr<ShaderData> newShader = std::make_shared<GLSLShader>(path, stage);
			_shaders.push_back(newShader);
			return newShader;

		}
		std::shared_ptr<const ShaderData> getOrCreateSPV(const char* shader, VkShaderStageFlagBits stage)
		{
			{
				std::shared_lock lock(_mutex);
				auto findShader = std::find_if(_shaders.begin(), _shaders.end(), [&](const std::shared_ptr<const ShaderData>& shaderPtr)
					{
						return shaderPtr->data() == shader && shaderPtr->stage() == stage;
					});
				if (findShader != _shaders.end())
				{
					return *findShader;
				}
			}

			std::unique_lock lock(_mutex);

			std::shared_ptr<ShaderData> newShader = std::make_shared<SPVShader>(shader, stage);
			_shaders.push_back(newShader);
			return newShader;

		}
		std::shared_ptr<const ShaderData> getOrCreateSPV(const std::filesystem::path& path, VkShaderStageFlagBits stage)
		{
			std::error_code ec;
			auto absolute = std::filesystem::absolute(path, ec);
			if (!std::filesystem::exists(absolute, ec))
				return nullptr;

			{
				std::shared_lock lock(_mutex);
				auto findShader = std::find_if(_shaders.begin(), _shaders.end(), [&](const std::shared_ptr<const ShaderData>& shaderPtr)
					{
						return shaderPtr->fileName() == path && shaderPtr->stage() == stage;
					});
				if (findShader != _shaders.end())
				{
					return *findShader;
				}
			}

			std::unique_lock lock(_mutex);

			std::shared_ptr<ShaderData> newShader = std::make_shared<SPVShader>(path, stage);
			_shaders.push_back(newShader);
			return newShader;

		}

	private:
		std::vector<std::shared_ptr<const ShaderData>> _shaders;
		std::shared_mutex _mutex;
	};

	ShaderCache& getShaderCache()
	{
		static ShaderCache cache;
		return cache;
	}



	PipelineDescription::PipelineDescription()
	{
	}

	PipelineDescription::~PipelineDescription()
	{
	}

	void PipelineDescription::addShaderGLSL(VkShaderStageFlagBits stage, const char* shader)
	{
		auto shaderData = getShaderCache().getOrCreateGLSL(shader, stage);
		_shaders.push_back({.stage = stage, .shader = shaderData});
	}

	void PipelineDescription::addShaderGLSL(VkShaderStageFlagBits stage, const std::filesystem::path& path)
	{
		auto shaderData = getShaderCache().getOrCreateGLSL(path, stage);
		_shaders.push_back({ .stage = stage, .shader = shaderData });
	}

	void PipelineDescription::addShaderSPV(VkShaderStageFlagBits stage, const char* shader)
	{
		auto shaderData = getShaderCache().getOrCreateSPV(shader, stage);
		_shaders.push_back({ .stage = stage, .shader = shaderData });
	}

	void PipelineDescription::addShaderSPV(VkShaderStageFlagBits stage, const std::filesystem::path& path)
	{
		auto shaderData = getShaderCache().getOrCreateSPV(path, stage);
		_shaders.push_back({ .stage = stage, .shader = shaderData });
	}

	void PipelineDescription::declareVertexAttribute(uint32_t binding, uint32_t location, VkFormat format, size_t bindingSize, size_t locationOffset)
	{
		_attributes.push_back({ .binding = binding, .location = location, .format = format, .size = bindingSize, .offset = locationOffset });
	}

	void PipelineDescription::declareUniform(uint32_t binding, size_t size)
	{
		_uniforms.push_back({ .binding = binding, .size = size });
	}

	void PipelineDescription::declarePushConstant(size_t size)
	{
		_pushConstant.size = size;
		_pushConstant.hasPushConstant = true;
	}

	void PipelineDescription::declareTexture(uint32_t binding)
	{
		_textures.push_back({ .binding = binding });
	}

	std::span<const PipelineDescription::ShaderDescription> PipelineDescription::shaders() const
	{
		return _shaders;
	}

	std::span<const PipelineDescription::VertexAttributeDescription> PipelineDescription::attributes() const
	{
		return _attributes;
	}

	std::span<const PipelineDescription::UniformDescription> PipelineDescription::uniforms() const
	{
		return _uniforms;
	}

	const PipelineDescription::PushConstantDescription& PipelineDescription::pushConstant() const
	{
		return _pushConstant;
	}

	std::span<const PipelineDescription::TextureDescription> PipelineDescription::textures() const
	{
		return _textures;
	}







	Pipeline::Pipeline(const Device& device, const PipelineDescription& description)
	{

	}

	VkPipeline Pipeline::handle() const
	{
		return _pipeline;
	}



}