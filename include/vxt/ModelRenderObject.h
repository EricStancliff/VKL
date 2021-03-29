#pragma once

#include <vkl/Common.h>  
#include <vxt/Model.h>
#include <vkl/RenderObject.h>  
#include <vkl/UniformBuffer.h>
#include <vxt/VXT_EXPORT.h>
#include <vkl/PipelineFactory.h>
#include <vxt/Camera.h>

namespace vxt
{
	class Camera;

	class VXT_EXPORT ModelShapeObject : public vkl::RenderObject
	{
		PIPELINE_TYPE
	public:
		static void describePipeline(vkl::PipelineDescription& description);

		ModelShapeObject() = delete;
		ModelShapeObject(const vkl::Device& device, const vkl::SwapChain& swapChain, const vkl::PipelineManager& pipelines, vkl::BufferManager& bufferManager);
		~ModelShapeObject() = default;
		ModelShapeObject(const ModelShapeObject&) = delete;
		ModelShapeObject& operator=(const ModelShapeObject&) = delete;
		ModelShapeObject(ModelShapeObject&&) noexcept = default;
		ModelShapeObject& operator=(ModelShapeObject&&) noexcept = default;

		void setShape(const vkl::Device& device, const vkl::SwapChain& swapChain, std::shared_ptr<const Model> model, size_t index);
		size_t getShape() const;

		void update(const vkl::Device& device, const vkl::SwapChain& swapChain, const glm::mat4& modelMatrix, const Camera& cam, std::shared_ptr<const Model> model,
			std::string_view animationName, double animationInput);


	private:

		struct MVP
		{
			glm::mat4 model{ glm::identity<glm::mat4>() };
			glm::mat4 view{ glm::identity<glm::mat4>() };
			glm::mat4 proj{ glm::identity<glm::mat4>() };
			glm::mat4 shape{ glm::identity<glm::mat4>() };
		};

		struct Joints
		{
			JointArray joints;
			float jointCount{ 0.f };
		};

		struct Lights
		{
			LightArray lights;
		};

		struct PBRMaterial
		{
			alignas(16) glm::vec4 baseColorFactor = glm::vec4(1.0f);
			alignas(4) float alphaCutoff = 1.0f;
			alignas(4) float metallicFactor = 1.0f;
			alignas(4) float roughnessFactor = 1.0f;

			//To be treated as mutually exclusive
			alignas(4) float alphaMode_opaque = 1.0f;
			alignas(4) float alphaMode_mask = 0.0f;
			alignas(4) float alphaMode_blend = 0.0f;
		};

		size_t _shapeIndex{ 0 };
		MVP _transform;
		Joints _joints;
		Lights _lights;
		PBRMaterial _material;

		std::shared_ptr<vkl::TypedUniform<MVP>> _uniform;
		std::shared_ptr<vkl::TypedUniform<Joints>> _jointsUniform;
		std::shared_ptr<vkl::TypedUniform<Lights>> _lightsUniform;
		std::shared_ptr<vkl::TypedUniform<PBRMaterial>> _materialUniform;
	};

	class VXT_EXPORT ModelRenderObject
	{
	public:
		ModelRenderObject() = default;
		~ModelRenderObject() = default;
		ModelRenderObject(const ModelRenderObject&) = delete;
		ModelRenderObject& operator=(const ModelRenderObject&) = delete;
		ModelRenderObject(ModelRenderObject&&) noexcept = default;
		ModelRenderObject& operator=(ModelRenderObject&&) noexcept = default;

		void setModel(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines, std::shared_ptr<const Model> model);
		std::shared_ptr<const Model> getModel() const;

		void animate(std::string_view animationName, double input);
		void update(const vkl::Device& device, const vkl::SwapChain& swapChain, const Camera& cam);

		glm::mat4 getTransform() const;
		void setTransform(const glm::mat4& transform);

		std::span<const std::shared_ptr<ModelShapeObject>> shapes() const;

	private:

		std::vector<std::shared_ptr<ModelShapeObject>> _shapes;
		std::shared_ptr<const Model> _model;
		glm::mat4 _transform{ glm::identity<glm::mat4>() };

		std::string _animationName;
		double _animationInput{ 0 };
	};
}