#pragma once
#include <vxt/VXT_EXPORT.h>
namespace vkl
{
	class Window;
}

namespace vxt
{
	class Camera;

	class VXT_EXPORT CameraManip
	{
	public:
		CameraManip() = default;
		virtual ~CameraManip() = default;
		CameraManip(const CameraManip&) = delete;
		CameraManip(CameraManip&&) noexcept = default;
		CameraManip& operator=(CameraManip&&) noexcept = default;
		CameraManip& operator=(const CameraManip&) = delete;



		virtual bool process(const vkl::Window& window, Camera& camera) = 0;
		virtual bool isFirstPersonNoMouseMode() const { return false; }

	};

}