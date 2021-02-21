#pragma once
#include <Common.h>
#include <Reflect.h>
#include <Pipeline.h>

#include <VertexBuffer.h>
#include <UniformBuffer.h>
#include <TextureBuffer.h>
#include <DrawCall.h>

namespace vkl
{

	class RenderObjectDescription : public reflect::reflection_data_base
	{
	public:
		PipelineDescription& pipelineDescription() {
			return _pipeline;
		}
		const PipelineDescription& pipelineDescription() const {
			return _pipeline;
		}
	private:
		PipelineDescription _pipeline;

	};

	class RenderObject : public reflect::object
	{
		REFLECTED_TYPE_CUSTOM(RenderObject, reflect::object, RenderObjectDescription)
		static void populateReflection(reflection_data& reflection);
	public:


		std::span< std::shared_ptr<VertexBuffer>> vbos();
		std::span< std::shared_ptr<UniformBuffer>> uniforms();
		std::span< std::shared_ptr<TextureBuffer>> textures();
		std::span< std::shared_ptr<DrawCall>> drawCalls();

	protected:
		void addVBO(std::shared_ptr<VertexBuffer> vbo);
		void addUniform(std::shared_ptr<UniformBuffer> uniform);
		void addTexture(std::shared_ptr<TextureBuffer>texture);
		void addDrawCall(std::shared_ptr<DrawCall> draw);

	private:
		std::vector<std::shared_ptr<VertexBuffer>> _vbos;
		std::vector<std::shared_ptr<UniformBuffer>> _uniforms;
		std::vector<std::shared_ptr<TextureBuffer>> _textures;
		std::vector<std::shared_ptr<DrawCall>> _drawCalls;
	};
}