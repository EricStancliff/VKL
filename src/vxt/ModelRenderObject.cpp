#include <vxt/ModelRenderObject.h>

#include <vxt/Model.h>
#include <vxt/Camera.h>
#include <vkl/BufferManager.h>

namespace
{
	constexpr const char* VertShader = R"Shader(

#version 450

layout(binding = 0) uniform MVP {
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 shape;
} u_mvp;

layout(binding = 1) uniform Joints {
	mat4 jointTransforms[128];
	float jointCount;
} u_joints;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec2 uv1;
layout(location = 4) in vec4 joint0;
layout(location = 5) in vec4 weight0;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV0;
layout (location = 2) out vec2 outUV1;

void main() {
    
	vec4 locPos;
	if (u_joints.jointCount > 0.0) {
		// Mesh is skinned
		mat4 skinMat = 
			weight0.x * u_joints.jointTransforms[int(joint0.x)] +
			weight0.y * u_joints.jointTransforms[int(joint0.y)] +
			weight0.z * u_joints.jointTransforms[int(joint0.z)] +
			weight0.w * u_joints.jointTransforms[int(joint0.w)];

		locPos = u_mvp.model * u_mvp.shape * skinMat * vec4(pos, 1.0);
		outNormal = normalize(transpose(inverse(mat3(u_mvp.model * u_mvp.shape * skinMat))) * normal);
	} else {
		locPos = u_mvp.model * u_mvp.shape * vec4(pos, 1.0);
		outNormal = normalize(transpose(inverse(mat3(u_mvp.model * u_mvp.shape))) * normal);
	}

	outUV0 = uv0;
	outUV1 = uv1;
	gl_Position =  u_mvp.proj * u_mvp.view * locPos;
}

)Shader";

	constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 2) uniform sampler2D texSampler;

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec2 uv1;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, uv0);
}

)Shader";

}

REGISTER_PIPELINE(vxt::ModelShapeObject, vxt::ModelShapeObject::describePipeline)

namespace {
	constexpr uint32_t _Binding_VBO = 0;

	constexpr uint32_t _Attribute_Pos = 0;
	constexpr uint32_t _Attribute_Norm = 1;
	constexpr uint32_t _Attribute_UV0 = 2;
	constexpr uint32_t _Attribute_UV1 = 3;
	constexpr uint32_t _Attribute_Joint0 = 4;
	constexpr uint32_t _Attribute_Weight0 = 5;

	constexpr uint32_t _Binding_MVP = 0;
	constexpr uint32_t _Binding_Joints = 1;
	constexpr uint32_t _Binding_BaseColorTexture = 2;
}

namespace vxt
{
	void ModelShapeObject::describePipeline(vkl::PipelineDescription& description)
	{
		description.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		description.addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		description.addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		description.declareVertexAttribute(_Binding_VBO, _Attribute_Pos, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, pos));
		description.declareVertexAttribute(_Binding_VBO, _Attribute_Norm, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, normal));
		description.declareVertexAttribute(_Binding_VBO, _Attribute_UV0, VK_FORMAT_R32G32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, uv0));
		description.declareVertexAttribute(_Binding_VBO, _Attribute_UV1, VK_FORMAT_R32G32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, uv1));
		description.declareVertexAttribute(_Binding_VBO, _Attribute_Joint0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, joint0));
		description.declareVertexAttribute(_Binding_VBO, _Attribute_Weight0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Model::Vertex), offsetof(Model::Vertex, weight0));
		
		
		description.declareUniform(_Binding_MVP, sizeof(MVP));
		description.declareUniform(_Binding_Joints, sizeof(Joints));


		description.declareTexture(_Binding_BaseColorTexture);
	}


	ModelShapeObject::ModelShapeObject(const vkl::Device& device, const vkl::SwapChain& swapChain, const vkl::PipelineManager& pipelines, vkl::BufferManager& bufferManager)
	{
		_uniform = bufferManager.createTypedUniform<MVP>(device, swapChain);
		_jointsUniform = bufferManager.createTypedUniform<Joints>(device, swapChain);
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

		addVBO(model->getVertexBuffer(), _Binding_VBO);
		addDrawCall(shape.draw);

		_transform.shape = shape.transform;
		addUniform(_uniform, _Binding_MVP);
		addUniform(_jointsUniform, _Binding_Joints);
		_jointsUniform->setData(_joints);

		if (shape.material >= 0 && shape.material < model->getMaterials().size())
		{
			addTexture(model->getMaterials()[shape.material].baseColorTexture, _Binding_BaseColorTexture);
		}
	}
	size_t ModelShapeObject::getShape() const
	{
		return _shapeIndex;
	}

	void ModelShapeObject::update(const vkl::Device& device, const vkl::SwapChain& swapChain, const glm::mat4& modelMatrix, const Camera& cam, std::shared_ptr<const Model> model, 
		std::string_view animationName, double animationInput)
	{
		_transform.model = modelMatrix;
		_transform.view = cam.view();
		_transform.proj = cam.projection();

		if (model->supportsAnimations())
		{
			if (model->animate(_joints.joints, _joints.jointCount, _transform.shape, _shapeIndex, animationName, animationInput))
			{
				_jointsUniform->setData(_joints);
			}
		}

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

	void ModelRenderObject::animate(std::string_view animationName, double input)
	{
		_animationName = std::string(animationName);
		_animationInput = input;
	}

	void ModelRenderObject::update(const vkl::Device& device, const vkl::SwapChain& swapChain, const Camera& cam)
	{
		for (auto&& shape : _shapes)
			shape->update(device, swapChain, _transform, cam, _model, _animationName, _animationInput);
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
