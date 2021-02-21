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

		std::span<const VertexBuffer> vbos() const;
		std::span<const UniformBuffer> uniforms() const;
		std::span<const TextureBuffer> textures() const;
		std::span<const DrawCall> drawCalls() const;

		std::span<VertexBuffer> vbos();
		std::span<UniformBuffer> uniforms();
		std::span<TextureBuffer> textures();
		std::span<DrawCall> drawCalls();

	protected:
		void addVBO(VertexBuffer&& vbo);
		void addUniform(UniformBuffer&& uniform);
		void addTexture(TextureBuffer&& texture);
		void addDrawCall(DrawCall&& draw);

	private:
		std::vector<VertexBuffer> _vbos;
		std::vector<UniformBuffer> _uniforms;
		std::vector<TextureBuffer> _textures;
		std::vector<DrawCall> _drawCalls;
	};
}