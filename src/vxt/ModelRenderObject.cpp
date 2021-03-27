#include <vxt/ModelRenderObject.h>

#include <vxt/Model.h>
#include <vxt/Camera.h>

namespace
{
	constexpr const char* VertShader = R"Shader(

#version 450

layout(push_constant) uniform MVP {
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
}

)Shader";

	constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragUV);
}

)Shader";

}

namespace vxt
{
	void ModelShapeObject::populateReflection(vkl::RenderObjectDescription& reflection)
	{
		reflection.pipelineDescription().setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		reflection.pipelineDescription().declareVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, pos));
		reflection.pipelineDescription().declareVertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, normal));
		reflection.pipelineDescription().declareVertexAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, uv0));

		reflection.pipelineDescription().declareTexture(1);
		reflection.pipelineDescription().declarePushConstant(sizeof(MVP));
	}

	void ModelShapeObject::init(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines)
	{
		vkl::RenderObject::init(device, swapChain, bufferManager, pipelines);
		_pushConstant = std::make_shared < vkl::PushConstant<MVP>>();
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

		addVBO(device, swapChain, model->getVertexBuffer(), 0);
		addDrawCall(device, swapChain, shape.draw);

		_pushConstant->typedData().shape = shape.transform;

		if (shape.material > 0 && shape.material <= model->getMaterials().size())
		{
			addTexture(device, swapChain, model->getMaterials()[shape.material].baseColorTexture, 1);
		}
	}
	size_t ModelShapeObject::getShape() const
	{
		return _shapeIndex;
	}

	void ModelShapeObject::update(const vkl::Device& device, const vkl::SwapChain& swapChain, const glm::mat4& model, const Camera& cam)
	{
		_pushConstant->typedData().model = model;
		_pushConstant->typedData().view = cam.view();
		_pushConstant->typedData().proj = cam.projection();
	}

	void ModelRenderObject::setModel(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines, std::shared_ptr<const Model> model)
	{
		_shapes.clear();

		_model = model;

		if (!model)
			return;

		for (int i = 0; i < model->getPrimitives().size(); ++i)
		{
			auto shape = std::make_shared<ModelShapeObject>();
			shape->init(device, swapChain, bufferManager, pipelines);
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

IMPL_REFLECTION(vxt::ModelShapeObject)
