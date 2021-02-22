#include <Pipeline.h>

#include <Shader.h>
#include <Reflect.h>
#include <RenderObject.h>
#include <Device.h>
#include <RenderPass.h>
#include <SwapChain.h>

#include <array>

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
	void PipelineDescription::setPrimitiveTopology(VkPrimitiveTopology topology)
	{
		_primitiveTopology = topology;
	}

	VkPrimitiveTopology PipelineDescription::primitiveTopology() const
	{
		return _primitiveTopology;
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
	/*****************************************************************************************************************/


	Pipeline::Pipeline(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass)
	{
		createDescriptorSetLayout(device, swapChain, description, renderPass);
		createPipeline(device, swapChain, description, renderPass);
	}

	void Pipeline::createDescriptorSetLayout(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass)
	{
		//Descriptor Pool
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain.framesInFlight());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChain.framesInFlight());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChain.framesInFlight());

		if (vkCreateDescriptorPool(device.handle(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
			//TODO - LOG
		}

		//Descriptor Set Layout
		std::vector< VkDescriptorSetLayoutBinding> layoutBindings;
		for (auto&& uniform : description.uniforms())
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = uniform.binding;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
			layoutBindings.push_back(uboLayoutBinding);
		}
		for (auto&& texture : description.textures())
		{
			VkDescriptorSetLayoutBinding texLayoutBinding{};
			texLayoutBinding.binding = texture.binding;
			texLayoutBinding.descriptorCount = 1;
			texLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texLayoutBinding.pImmutableSamplers = nullptr;
			texLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
			layoutBindings.push_back(texLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(device.handle(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
			//TODO - LOG
		}

	}

	void Pipeline::createPipeline(const Device& device, const SwapChain& swapChain, const PipelineDescription& description, const RenderPass& renderPass)
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
		std::vector<ShaderModule> shaderModules;

		for (auto&& shader : description.shaders())
		{
			if (!shader.shader)
				continue;

			ShaderModule shaderMod(device, shader.shader, shader.stage);

			VkPipelineShaderStageCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			createInfo.stage = shader.stage;
			createInfo.module = shaderMod.handle();
			createInfo.pName = "main";

			shaderModules.emplace_back(std::move(shaderMod));
			shaderStageCreateInfos.emplace_back(std::move(createInfo));

		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::vector< VkVertexInputBindingDescription> bindingDescriptions;
		std::vector< VkVertexInputAttributeDescription> attributeDescriptions;

		for (auto&& vbo : description.attributes())
		{
			auto findBinding = std::find_if(bindingDescriptions.begin(), bindingDescriptions.end(), [&](const VkVertexInputBindingDescription& rhs) {
				return vbo.binding == rhs.binding;
				});

			if (findBinding == bindingDescriptions.end())
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = vbo.binding;
				bindingDescription.stride = (uint32_t)vbo.size;
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				bindingDescriptions.emplace_back(std::move(bindingDescription));
			}

			VkVertexInputAttributeDescription attributeDesc{};
			attributeDesc.binding = vbo.binding;
			attributeDesc.location = vbo.location;
			attributeDesc.format = vbo.format;
			attributeDesc.offset = (uint32_t)vbo.offset;
			attributeDescriptions.emplace_back(std::move(attributeDesc));

		}


		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = description.primitiveTopology();
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = device.maxUsableSamples();

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates;
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);


		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

		if (vkCreatePipelineLayout(device.handle(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
			//TODO - LOG
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineInfo.pStages = shaderStageCreateInfos.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = renderPass.handle();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS) {
			//TODO - LOG
		}

		for (auto&& shaderMod : shaderModules)
			vkDestroyShaderModule(device.handle(), shaderMod.handle(), nullptr);


	}

	VkDescriptorPool Pipeline::descriptorPoolHandle() const
	{
		return _descriptorPool;
	}

	VkDescriptorSetLayout Pipeline::descriptorSetLayoutHandle() const
	{
		return _descriptorSetLayout;
	}

	VkPipeline Pipeline::handle() const
	{
		return _pipeline;
	}

	size_t Pipeline::type() const
	{
		return _type;
	}
	VkPipelineLayout Pipeline::pipelineLayoutHandle() const
	{
		return _pipelineLayout;
	}
	/*****************************************************************************************************************/
	PipelineManager::PipelineManager(const Device& device, const SwapChain& swapChain, const RenderPass& renderPass)
	{
		reflect::forAllTypesDerivedFrom<RenderObject>([&](const reflect::reflection& data) {
			const RenderObjectDescription* roDesc = dynamic_cast<const RenderObjectDescription*>(data.data);
			if (!roDesc)
				return;

			const PipelineDescription& pipelineDesc = roDesc->pipelineDescription();
			size_t typeIndex = data.index;

			auto findWhere = std::lower_bound(_pipelines.begin(), _pipelines.end(), typeIndex, [&](const Pipeline& pipePair, const size_t& index) {
				return pipePair.type() < index;
				});

			Pipeline pipeline(device, swapChain, pipelineDesc, renderPass);
			_pipelines.emplace_back(std::move(pipeline));

		});

	}
	const Pipeline* PipelineManager::pipelineForType(size_t type) const
	{
		auto findWhere = std::lower_bound(_pipelines.begin(), _pipelines.end(), type, [&](const Pipeline& pipePair, const size_t& index) {
			return pipePair.type() < index;
			});
		if (findWhere == _pipelines.end())
			return nullptr;
		return &(*findWhere);
	}

}