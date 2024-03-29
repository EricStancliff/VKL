#include <vxt/FirstPersonManip.h>
#include <vkl/Window.h>
#include <vxt/Camera.h>

namespace vxt
{
	bool FirstPersonManip::process(const vkl::Window& window, Camera& camera)
	{

		if (!_init)
		{
			updateProjection(window, camera);
			_init = true;
		}

		bool updated = false;
		for (auto&& event : window.events())
		{
			if (event->getType() == vkl::EventType::WINDOW_RESIZE)
			{
				updateProjection(window, camera);
				updated = true;
			}
			else if (event->getType() == vkl::EventType::KEY_DOWN)
			{
				_keysDown.insert(static_cast<const vkl::KeyDownEvent*>(event.get())->key);
			}
			else if (event->getType() == vkl::EventType::KEY_UP)
			{
				_keysDown.erase(static_cast<const vkl::KeyDownEvent*>(event.get())->key);
			}
			else if (event->getType() == vkl::EventType::MOUSE_DOWN)
			{
				_leftMouseDown |= static_cast<const vkl::MouseDownEvent*>(event.get())->button == vkl::MouseButton::LEFT;
				_rightMouseDown |= static_cast<const vkl::MouseDownEvent*>(event.get())->button == vkl::MouseButton::RIGHT;
			}
			else if (event->getType() == vkl::EventType::MOUSE_UP)
			{
				if(_leftMouseDown && static_cast<const vkl::MouseUpEvent*>(event.get())->button == vkl::MouseButton::LEFT)
					_leftMouseDown = false;
				if (_rightMouseDown && static_cast<const vkl::MouseUpEvent*>(event.get())->button == vkl::MouseButton::RIGHT)
					_rightMouseDown = false;
			}
			else if (event->getType() == vkl::EventType::MOUSE_MOVE)
			{
				auto mouseEvent = static_cast<const vkl::MouseMoveEvent*>(event.get());
				_mousePos.x = (float)mouseEvent->x;
				_mousePos.y = (float)mouseEvent->y;
			}

		}

		updated |= rotate(window, camera);
		updated |= translate(window, camera);

		_lastMousePos = _mousePos;

		return updated;
	}

	float FirstPersonManip::speed() const
	{
		return _speed;
	}

	void FirstPersonManip::setSpeed(float speed)
	{
		_speed = speed;
	}

	void FirstPersonManip::enableHeadlight(size_t index)
	{
		_headlight = index;
	}

	void FirstPersonManip::disableHeadlight()
	{
		_headlight = std::numeric_limits<size_t>::max();
	}

	bool FirstPersonManip::headlightEnabled() const
	{
		return _headlight != std::numeric_limits<size_t>::max();
	}
	size_t FirstPersonManip::headlight() const
	{
		return _headlight;
	}

	bool FirstPersonManip::isFirstPersonNoMouseMode() const
	{
		return _leftMouseDown || _rightMouseDown;
	}

	bool FirstPersonManip::rotate(const vkl::Window& window, Camera& camera)
	{
		if (_leftMouseDown && _lastMousePos != _mousePos)
		{
			float dx = _mousePos.x - _lastMousePos.x;
			float dy = _mousePos.y - _lastMousePos.y;

			dx *= .25;
			dy *= .25;

			auto view = camera.view();
			auto invView = glm::inverse(view);

			invView = glm::rotate(invView, glm::radians(dx), { 0, -1, 0 });
			invView = glm::rotate(invView, glm::radians(dy), { -1, 0, 0 });

			view = glm::inverse(invView);

			camera.setView(view);
			updateProjection(window, camera);

			return true;
		}
		if (_rightMouseDown && _lastMousePos != _mousePos)
		{
			float dx = _mousePos.x - _lastMousePos.x;

			dx *= .25;

			auto view = camera.view();
			auto invView = glm::inverse(view);

			invView = glm::rotate(invView, glm::radians(dx), { 0, 0, -1 });

			view = glm::inverse(invView);

			camera.setView(view);
			updateProjection(window, camera);

			return true;
		}
		return false;
	}

	bool FirstPersonManip::translate(const vkl::Window& window, Camera& camera)
	{
		bool move = false;

		auto view = camera.view();

		glm::vec3 trans = glm::inverse(view)[3];

		glm::vec3 right;
		right.x = view[0][0];
		right.y = view[1][0];
		right.z = view[2][0];

		glm::vec3 up;
		up.x = view[0][1];
		up.y = view[1][1];
		up.z = view[2][1];

		glm::vec3 forward;
		forward.x = -view[0][2];
		forward.y = -view[1][2];
		forward.z = -view[2][2];

		for (auto&& key : _keysDown)
		{
			if (key == vkl::Key::KEY_W)
			{
				//forward
				trans = trans + (_speed * forward);
				move = true;
			}
			else if (key == vkl::Key::KEY_A)
			{
				//left
				trans = trans - (_speed * right);
				move = true;
			}
			else if (key == vkl::Key::KEY_S)
			{
				//back
				//forward
				trans = trans - (_speed * forward);
				move = true;
			}
			else if (key == vkl::Key::KEY_D)
			{
				//right
				trans = trans + (_speed * right);
				move = true;
			}
			else if (key == vkl::Key::KEY_Q)
			{
				//down
				trans = trans - (_speed * up);
				move = true;
			}
			else if (key == vkl::Key::KEY_E)
			{
				//up
				trans = trans + (_speed * up);
				move = true;
			}

		}

		if (move)
		{
			camera.setView(glm::lookAt(trans, trans + forward, up));
			updateProjection(window, camera);
		}

		return move;
	}

	void FirstPersonManip::updateProjection(const vkl::Window& window, Camera& camera)
	{
		auto proj = glm::perspective(glm::radians(45.f), (float)window.width() / (float)window.height(), 0.1f, 1000.f);
		proj[1][1] *= -1;
		camera.setProjection(proj); //TEMP

		if (headlightEnabled())
		{
			Light light;
			light.attenuate = 0.f;
			light.power = 1.f;
			light.color = glm::one<glm::vec4>();

			light.position = {};

			camera.setLight(_headlight, light);
		}
	}


}