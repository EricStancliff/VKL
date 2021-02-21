#include <RenderObject.h>

namespace vkl
{
	void RenderObject::populateReflection(reflection_data& reflection)
	{
	}


	std::span<std::shared_ptr<VertexBuffer>> RenderObject::vbos()
	{
		return _vbos;
	}

	std::span<std::shared_ptr<UniformBuffer>> RenderObject::uniforms()
	{
		return _uniforms;
	}

	std::span<std::shared_ptr<TextureBuffer>> RenderObject::textures()
	{
		return _textures;
	}

	std::span<std::shared_ptr<DrawCall>> RenderObject::drawCalls()
	{
		return _drawCalls;
	}

	void RenderObject::addVBO(std::shared_ptr<VertexBuffer> vbo)
	{
		_vbos.emplace_back(std::move(vbo));
	}
	void RenderObject::addUniform(std::shared_ptr<UniformBuffer> uniform)
	{
		_uniforms.emplace_back(std::move(uniform));
	}
	void RenderObject::addTexture(std::shared_ptr<TextureBuffer> texture)
	{
		_textures.emplace_back(std::move(texture));
	}
	void RenderObject::addDrawCall(std::shared_ptr<DrawCall> draw)
	{
		_drawCalls.emplace_back(std::move(draw));
	}
}

IMPL_REFLECTION(vkl::RenderObject)