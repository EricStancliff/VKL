#include <vxt/Camera.h>

namespace vxt
{
	glm::mat4 Camera::view() const
	{
		return _view;
	}
	void Camera::setView(const glm::mat4& view)
	{
		_view = view;
	}
	glm::mat4 Camera::projection() const
	{
		return _proj;
	}
	void Camera::setProjection(const glm::mat4& projection)
	{
		_proj = projection;
	}
	glm::vec4 Camera::viewport() const
	{
		return _viewport;
	}
	void Camera::setViewport(const glm::vec4& viewport)
	{
		_viewport = viewport;
	}
	void Camera::setLight(size_t lightIndex, const Light& light)
	{
		assert(lightIndex < MaxNumLights && "Light index too high!!");
		_lights[lightIndex] = light;
	}
	void Camera::clearLights()
	{
		_lights.fill({});
	}
	std::span<const Light> Camera::lights() const
	{
		return _lights;
	}
}