#include <RenderObject.h>

namespace vkl
{
	void RenderObject::populateReflection(reflection_data& reflection)
	{
	}
	std::span<const VertexBuffer> RenderObject::vbos() const
	{
		return _vbos;
	}
	std::span<const UniformBuffer> RenderObject::uniforms() const
	{
		return _uniforms;
	}
	std::span<const TextureBuffer> RenderObject::textures() const
	{
		return _textures;
	}
	std::span<const DrawCall> RenderObject::drawCalls() const
	{
		return _drawCalls;
	}
	std::span<VertexBuffer> RenderObject::vbos() 
	{
		return _vbos;
	}
	std::span<UniformBuffer> RenderObject::uniforms() 
	{
		return _uniforms;
	}
	std::span<TextureBuffer> RenderObject::textures() 
	{
		return _textures;
	}
	std::span<DrawCall> RenderObject::drawCalls() 
	{
		return _drawCalls;
	}
	void RenderObject::addVBO(VertexBuffer&& vbo)
	{
		_vbos.emplace_back(std::move(vbo));
	}
	void RenderObject::addUniform(UniformBuffer&& uniform)
	{
		_uniforms.emplace_back(std::move(uniform));
	}
	void RenderObject::addTexture(TextureBuffer&& texture)
	{
		_textures.emplace_back(std::move(texture));
	}
	void RenderObject::addDrawCall(DrawCall&& draw)
	{
		_drawCalls.emplace_back(std::move(draw));
	}
}

IMPL_REFLECTION(vkl::RenderObject)