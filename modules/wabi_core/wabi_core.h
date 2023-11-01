/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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