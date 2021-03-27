#pragma once

#include <vkl/Common.h>  
#include <vxt/Model.h>
#include <vkl/RenderObject.h>  
#include <vkl/UniformBuffer.h>
#include <vxt/VXT_EXPORT.h>
#include <vkl/PipelineFactory.h>

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

		//This gets any bigger and we can't push constant
		struct MVP
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
			glm::mat4 shape;
		};
		void update(const vkl::Device& device, const vkl::SwapChain& swapChain, const glm::mat4& model, const Camera& cam);


	private:
		size_t _shapeIndex{ 0 };
		MVP _transform;
		std::shared_ptr<vkl::TypedUniform<MVP>> _uniform;
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

		void update(const vkl::Device& device, const vkl::SwapChain& swapChain, const Camera& cam);

		glm::mat4 getTransform() const;
		void setTransform(const glm::mat4& transform);

		std::span<const std::shared_ptr<ModelShapeObject>> shapes() const;

	private:

		std::vector<std::shared_ptr<ModelShapeObject>> _shapes;
		std::shared_ptr<const Model> _model;
		glm::mat4 _transform{ glm::identity<glm::mat4>() };
	};
}