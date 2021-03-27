#pragma once

#include <vxt/LinearAlgebra.h>

namespace vxt
{

	class Camera
	{
	public:
		Camera() = default;
		~Camera() = default;
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
		Camera(Camera&&) noexcept = default;
		Camera& operator=(Camera&&) noexcept = default;

		glm::mat4 view() const;
		void setView(const glm::mat4& view);

		glm::mat4 projection() const;
		void setProjection(const glm::mat4& projection);

		glm::vec4 viewport() const;
		void setViewport(const glm::vec4& viewport);

	private:
		glm::mat4 _view;
		glm::mat4 _proj;
		glm::vec4 _viewport;
	};
}