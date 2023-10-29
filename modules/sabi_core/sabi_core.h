#pragma once

#include "../wabi_core/wabi_core.h"
#include <rapidobj/rapidobj.hpp>

constexpr float DEFAULT_ZOOM_FACTOR = 0.5f;
constexpr float DEFAULT_ZOOM_MULTIPLIER = 200.0f;

const Eigen::Vector3f DEFAULT_CAMERA_POSIIION = Eigen::Vector3f (2.0f, 3.0f, -4.0f);
const Eigen::Vector3f DEFAULT_CAMERA_TARGET = Eigen::Vector3f::Zero();
constexpr float DEFAULT_FOV_DEGREES = 45.0f;
constexpr float DEFAULT_ASPECT = (float)DEFAULT_DESKTOP_WINDOW_WIDTH / (float)DEFAULT_DESKTOP_WINDOW_HEIGHT;
constexpr float DEFAULT_NEAR_PLANE = 0.01f;
constexpr float DEFAULT_FAR_PLANE = 1000.0f;
constexpr float DEFAULT_FOCAL_LENGTH = 1.0f;
constexpr float DEFAULT_APETURE = 0.0f;

namespace sabi
{
	// camera
	#include "excludeFromBuild/camera/CameraSensor.h"
	#include "excludeFromBuild/camera/CameraBody.h"

} // namespace wabi