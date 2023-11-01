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

template <class T>
inline bool isOrthogonal (const Eigen::Matrix<T, 3, 3>& m)
{
    // to test whether a matrix is orthogonal, mutliply the matrix by
    // it's transform and compare to identity matrix
    return (m * m.transpose()).isIdentity();
}

inline bool reOrthogonalize (Eigen::Matrix3f& m)
{
    // http://stackoverflow.com/questions/23080791/eigen-re-orthogonalization-of-rotation-matrix

    Eigen::Matrix3f mo = m;

    Eigen::Vector3f x = mo.row (0);
    Eigen::Vector3f y = mo.row (1);
    Eigen::Vector3f z = mo.row (2);

    float error = x.dot (y);

    Eigen::Vector3f x_ort = x - (error / 2) * y;
    Eigen::Vector3f y_ort = y - (error / 2) * x;
    Eigen::Vector3f z_ort = x_ort.cross (y_ort);

    mo.row (0) = x_ort.normalized();
    mo.row (1) = y_ort.normalized();
    mo.row (2) = z_ort.normalized();

    if (isOrthogonal (mo))
    {
        m = mo;
        return true;
    }
    else
    {
        return false;
    }
}

// from Instant Meshes
inline float fast_acos (float x)
{
    float negate = float (x < 0.0f);
    x = std::abs (x);
    float ret = -0.0187293f;
    ret *= x;
    ret = ret + 0.0742610f;
    ret *= x;
    ret = ret - 0.2121144f;
    ret *= x;
    ret = ret + 1.5707288f;
    ret = ret * std::sqrt (1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * (float)M_PI + ret;
}

// https://liuzhiguang.wordpress.com/2017/06/12/find-the-angle-between-two-vectors/
inline float angleBetweenVectors (Eigen::Vector3f a, Eigen::Vector3f b)
{
    float angle = 0.0f;

    angle = std::atan2 (a.cross (b).norm(), a.dot (b));

    return angle;
}

// rad_to_deg
template <typename T>
inline T rad_to_deg (const T rad)
{
    return rad * 180 / Math<T>::PI;
}

// deg_to_rad
template <typename T>
inline T deg_to_rad (const T deg)
{
    return deg * Math<T>::PI / 180;
}