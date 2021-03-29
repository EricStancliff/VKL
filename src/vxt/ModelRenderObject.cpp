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
layout (location = 3) out vec3 outViewPosition;

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
		outNormal = normalize(mat3(u_mvp.view * u_mvp.model * u_mvp.shape * skinMat) * normal);
	} else {
		locPos = u_mvp.model * u_mvp.shape * vec4(pos, 1.0);
		outNormal = normalize(mat3(u_mvp.view * u_mvp.model * u_mvp.shape) * normal);
	}

	outUV0 = uv0;
	outUV1 = uv1;
	outViewPosition = (u_mvp.view * locPos).xyz;
	gl_Position =  u_mvp.proj * u_mvp.view * locPos;
}

)Shader";

	//Frag shader logic (mostly) taken from OpenGL 4 Shading Language Cookbook Third Edition
    //https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition/blob/master/chapter04/shader/pbr.frag.glsl#L21
	constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 2) uniform sampler2D baseColorSampler;

layout(binding = 0) uniform MVP {
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 shape;
} u_mvp;

struct Light
{
	vec3 position;
	vec3 color;
	float power;
	float attenuate;
};

#define MaxLights 8

layout(binding = 3) uniform Lights {
	Light[MaxLights] lights;
} u_lights;

layout(binding = 4) uniform PBRMaterial {
	vec4 baseColorFactor;
	float alphaCutoff;
	float metallicFactor;
	float roughnessFactor;
	float alphaMode_opaque;
	float alphaMode_mask;
	float alphaMode_blend;
} u_material;


layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec2 uv1;
layout (location = 3) in vec3 viewPosition;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265358979323846;

vec4 baseColor()
{
	return texture(baseColorSampler, uv0) * u_material.baseColorFactor; 
}

float ggxDistribution( float nDotH ) {
  float alpha2 = u_material.roughnessFactor * u_material.roughnessFactor * u_material.roughnessFactor * u_material.roughnessFactor;
  float d = (nDotH * nDotH) * (alpha2 - 1) + 1;
  return alpha2 / (PI * d * d);
}

float geomSmith( float dotProd ) {
  float k = (u_material.roughnessFactor + 1.0) * (u_material.roughnessFactor + 1.0) / 8.0;
  float denom = dotProd * (1 - k) + k;
  return 1.0 / denom;
}

vec3 schlickFresnel( float lDotH, vec3 baseColor ) {
  vec3 f0 = mix(vec3(0.04), baseColor, u_material.metallicFactor);
  return f0 + (1 - f0) * pow(1.0 - lDotH, 5);
}

vec3 microfacetModel( int lightIdx, vec3 position, vec3 n ) {  

  vec3 baseColor = baseColor().xyz;
  vec3 diffuseBrdf = mix(baseColor, vec3(0.04), u_material.metallicFactor);

  vec3 l = vec3(0.0); 
  vec3 lightI = vec3(u_lights.lights[lightIdx].power) * u_lights.lights[lightIdx].color;  //"intensity"

  vec3 lightPosition = (u_mvp.view * u_mvp.model * vec4(u_lights.lights[lightIdx].position, 1.f)).xyz;

  l = lightPosition - position;
  float dist = length(l);
  l = normalize(l);
  
  if(u_lights.lights[lightIdx].attenuate > 0.f)
  {
    lightI /= (dist * dist);
  }

  vec3 v = normalize( -position );
  vec3 h = normalize( v + l );
  float nDotH = dot( n, h );
  float lDotH = dot( l, h );
  float nDotL = max( dot( n, l ), 0.0 );
  float nDotV = dot( n, v );
  vec3 specBrdf = 0.25 * ggxDistribution(nDotH) * schlickFresnel(lDotH, baseColor) * geomSmith(nDotL) * geomSmith(nDotV);

  return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
}

void main() {
	vec3 sum = vec3(0);
	for( int i = 0; i < MaxLights; i++ ) {
		sum += microfacetModel(i, viewPosition, normal);
	}
	outColor = vec4(sum, 1);
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
	constexpr uint32_t _Binding_Lights = 3;
	constexpr uint32_t _Binding_Material = 4;
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
		description.declareUniform(_Binding_Lights, sizeof(Lights));
		description.declareUniform(_Binding_Material, sizeof(PBRMaterial));


		description.declareTexture(_Binding_BaseColorTexture);
	}


	ModelShapeObject::ModelShapeObject(const vkl::Device& device, const vkl::SwapChain& swapChain, const vkl::PipelineManager& pipelines, vkl::BufferManager& bufferManager)
	{
		_uniform = bufferManager.createTypedUniform<MVP>(device, swapChain);
		_jointsUniform = bufferManager.createTypedUniform<Joints>(device, swapChain);
		_lightsUniform = bufferManager.createTypedUniform<Lights>(device, swapChain);
		_materialUniform = bufferManager.createTypedUniform<PBRMaterial>(device, swapChain);
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
		_jointsUniform->setData(_joints);
		addUniform(_jointsUniform, _Binding_Joints);
		_lightsUniform->setData(_lights);
		addUniform(_lightsUniform, _Binding_Lights);
		if (shape.material >= 0 && shape.material < model->getMaterials().size())
		{
			addTexture(model->getMaterials()[shape.material].baseColorTexture, _Binding_BaseColorTexture);

			const auto& mat = model->getMaterials()[shape.material];
			_material.alphaCutoff = mat.alphaCutoff;
			_material.alphaMode_blend = mat.alphaMode == Model::Material::AlphaMode::ALPHAMODE_BLEND;
			_material.alphaMode_mask = mat.alphaMode == Model::Material::AlphaMode::ALPHAMODE_MASK;
			_material.alphaMode_opaque = mat.alphaMode == Model::Material::AlphaMode::ALPHAMODE_OPAQUE;
			_material.baseColorFactor = mat.baseColorFactor;
			_material.metallicFactor = mat.metallicFactor;
			_material.roughnessFactor = mat.roughnessFactor;
		}
		_materialUniform->setData(_material);
		addUniform(_materialUniform, _Binding_Material);

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

		for (int i = 0; i < cam.lights().size(); ++i)
		{
			_lights.lights[i] = cam.lights()[i];
		}
		_lightsUniform->setData(_lights);

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
