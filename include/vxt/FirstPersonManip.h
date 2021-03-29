#pragma once

#include <vxt/CameraManip.h>
#include <set>
#include <vkl/Event.h>
#include <vxt/LinearAlgebra.h>

namespace vxt
{

	class VXT_EXPORT FirstPersonManip : public CameraManip
	{
	public:
		FirstPersonManip() = default;
		virtual ~FirstPersonManip() = default;
		FirstPersonManip(const FirstPersonManip&) = delete;
		FirstPersonManip(FirstPersonManip&&) noexcept = default;
		FirstPersonManip& operator=(FirstPersonManip&&) noexcept = default;
		FirstPersonManip& operator=(const FirstPersonManip&) = delete;

		virtual bool process(const vkl::Window& window, Camera& camera) override;

		float speed() const;
		void setSpeed(float speed);

		void enableHeadlight(size_t index);
		void disableHeadlight();
		bool headlightEnabled() const;
		size_t headlight() const;

	private:

		void updateProjection(const vkl::Window& window, Camera& camera);
		bool rotate(const vkl::Window& window, Camera& camera);
		bool translate(const vkl::Window& window, Camera& camera);

		std::set<vkl::Key> _keysDown;
		glm::vec2 _mousePos;
		glm::vec2 _lastMousePos;

		float _speed{ 1.f };
		bool _leftMouseDown{ false };
		bool _rightMouseDown{ false };

		bool _init{ false };
		size_t _headlight{ std::numeric_limits<size_t>::max() };
	};
}