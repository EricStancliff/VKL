#include <vxt/ModelRenderObject.h>

#include <vxt/Model.h>
#include <vxt/Camera.h>
#include <vkl/BufferManager.h>

namespace
{
	constexpr const char* VertShader = R"Shader(

#version 450

layout(binding = 2) uniform MVP {
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 shape;
} u_mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = u_mvp.proj * u_mvp.view * u_mvp.model * u_mvp.shape * vec4(inPosition, 1);
	fragUV = inUV;
}

)Shader";

	constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragUV);
	//outColor = vec4(1, 1, 1, 1);
}

)Shader";

}

REGISTER_PIPELINE(vxt::ModelShapeObject, vxt::ModelShapeObject::describePipeline)

namespace vxt
{
	void ModelShapeObject::describePipeline(vkl::PipelineDescription& description)
	{
		description.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		description.addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		description.addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		description.declareVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, pos));
		description.declareVertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, normal));
		description.declareVertexAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, uv0));

		description.declareTexture(1);
		description.declareUniform(2, sizeof(MVP));
	}

	ModelShapeObject::ModelShapeObject(const vkl::Device& device, const vkl::SwapChain& swapChain, const vkl::PipelineManager& pipelines, vkl::BufferManager& bufferManager)
	{
		_uniform = bufferManager.createTypedUniform<MVP>(device, swapChain);
	}

	void ModelShapeObject::setShape(const vkl::Device& device, const vkl::SwapChain& swapChain, std::shared_ptr<const Model> model, size_t index)
	{
		if (!model)
			return;
		if (index >= model->getPrimitives().size())
			return;

		_shapeIndex = index;
	
		reset();

		const auto& shape = model->getPrimitives()[index];

		addVBO(model->getVertexBuffer(), 0);
		addDrawCall(shape.draw);

		_transform.shape = shape.transform;
		addUniform(_uniform, 2);

		if (shape.material >= 0 && shape.material < model->getMaterials().size())
		{
			addTexture(model->getMaterials()[shape.material].baseColorTexture, 1);
		}
	}
	size_t ModelShapeObject::getShape() const
	{
		return _shapeIndex;
	}

	void ModelShapeObject::update(const vkl::Device& device, const vkl::SwapChain& swapChain, const glm::mat4& model, const Camera& cam)
	{
		_transform.model = model;
		_transform.view = cam.view();
		_transform.proj = cam.projection();
		_uniform->setData(_transform);
	}

	void ModelRenderObject::setModel(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines, std::shared_ptr<const Model> model)
	{
		_shapes.clear();

		_model = model;

		if (!model)
			return;

		for (int i = 0; i < model->getPrimitives().size(); ++i)
		{
			auto shape = std::make_shared<ModelShapeObject>(device, swapChain, pipelines, bufferManager);
			shape->setShape(device, swapChain, model, i);
			_shapes.push_back(shape);
		}
	}

	std::shared_ptr<const Model> ModelRenderObject::getModel() const
	{
		return _model;
	}

	void ModelRenderObject::update(const vkl::Device& device, const vkl::SwapChain& swapChain, const Camera& cam)
	{
		for (auto&& shape : _shapes)
			shape->update(device, swapChain, _transform, cam);
	}

	glm::mat4 ModelRenderObject::getTransform() const
	{
		return _transform;
	}

	void ModelRenderObject::setTransform(const glm::mat4& transform)
	{
		_transform = transform;
	}
	std::span<const std::shared_ptr<ModelShapeObject>> ModelRenderObject::shapes() const
	{
		return _shapes;
	}

}
