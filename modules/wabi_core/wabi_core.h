#pragma once

#include "../mace_core/mace_core.h"

using Eigen::Matrix;
using Pose = Eigen::Affine3f;
using Scale = Eigen::Vector3f;

using Float = float;
using MatrixXc = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXf = Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3u = Eigen::Matrix<uint32_t, 3, 1>;
using Vector4u = Eigen::Matrix<uint32_t, 4, 1>;
using Vector2u = Eigen::Matrix<uint32_t, 2, 1>;
// using Matrix3f = Eigen::Matrix<Float, 3, 3>;
using Matrix43f = Eigen::Matrix<Float, 4, 3>;
using MatrixRowMajor34f = Eigen::Matrix<Float, 3, 4, Eigen::RowMajor>;
using Matrix4f = Eigen::Matrix<Float, 4, 4>;
using MatrixXu16 = Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>;
using VectorXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, 1>;
using VectorXb = Eigen::Matrix<bool, Eigen::Dynamic, 1>;
using MatrixXi = Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic>;

namespace wabi
{
	// math
	#include "excludeFromBuild/math/Ray3.h"
	#include "excludeFromBuild/math/Plane.h"
	#include "excludeFromBuild/math/Maths.h"
	#include "excludeFromBuild/math/MathUtil.h"

} // namespace wabi