#pragma once

#include <vxt/LinearAlgebra.h>
#include <vxt/VXT_EXPORT.h>
#include <array>
#include <span>

namespace vxt
{
	constexpr size_t MaxNumLights = 8u;
	struct Light
	{
		alignas(16) glm::vec3 position{};
		alignas(16) glm::vec3 color{};
		alignas(4) float power{ 0.f };
		alignas(4) float attenuate{ 0.f };
	};

	using LightArray = Light[MaxNumLights];

	class VXT_EXPORT Camera
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

		void setLight(size_t lightIndex, const Light& light);
		void clearLights();
		std::span<const Light> lights() const;

	private:
		glm::mat4 _view;
		glm::mat4 _proj;
		glm::vec4 _viewport;

		std::array<Light, MaxNumLights>  _lights;
	};
}