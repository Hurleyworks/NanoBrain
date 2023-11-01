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

// from https://www.geometrictools.com/

template <typename T>
struct Ray3
	/// Represents a oriented line segment in 3d space
	/// The direction is normalized
	/// The valid region is delimited by 2
	/// distances along the ray, tMin and tMax
{
	Ray3 ();
	Ray3 (const Eigen::Matrix<T,3,1>  & o, 
          const Eigen::Matrix<T,3,1>  & d, 
          T start = std::numeric_limits<T>::epsilon(),
		  T end = std::numeric_limits<T>::max());

	~Ray3() = default;
	
	Eigen::Matrix<T, 3, 1> operator () (T t) const { return origin + dir * t; }

	T makeRayBetweenPoints (const Eigen::Matrix<T,3,1>  & point1, 
						    const Eigen::Matrix<T,3,1>  & point2);
	
	Eigen::Matrix<T,3,1> origin, dir;
	T tMin, tMax;

	Eigen::Matrix<T, 3, 1> hitPoint;
	Eigen::Matrix<T, 3, 1> surfaceNormal;

	void resetToDefault();

	BodyID hitBodyID;
	PolyID hitPolyID;
	T distToHit;
	bool wasHit;
	T bary0, bary1, bary2;

}; // end typename Ray3

// some useful typedefs
typedef Ray3<float> Ray3f;
typedef Ray3<double> Ray3d;
typedef std::vector<Ray3f> RayBundlef;
typedef std::vector<Ray3d> RayBundled;

// ctor
template <typename T>
Ray3<T>::Ray3 ()
	: origin(),
	  dir(),
	  tMin(std::numeric_limits<T>::epsilon()),
	  tMax(std::numeric_limits<T>::max()),
	  hitBodyID(INVALID_ID),
	  hitPolyID(INVALID_ID),
	  distToHit(std::numeric_limits<T>::max()),
	  wasHit(false)
{	
	bary0 = bary1 = bary2 = std::numeric_limits<T>::max();
	dir.normalize();

	hitPoint = Eigen::Matrix<T, 3, 1>::Constant(std::numeric_limits<T>::max());
	surfaceNormal = Eigen::Matrix<T, 3, 1>::Constant(0.0);
}

 //ctor
template <typename T>
Ray3<T>::Ray3 (const Eigen::Matrix<T,3,1>  & o, 
			   const Eigen::Matrix<T,3,1>  & d, 
			   T start, 
			   T end )
   : origin(o),
	 dir(d),
	 tMin(start),
	 tMax(end),
	 hitBodyID(INVALID_ID),
	 hitPolyID(INVALID_ID),
	 distToHit(std::numeric_limits<T>::max()),
	 wasHit(false)
{	
	bary0 = bary1 = bary2 = std::numeric_limits<T>::max();
	dir.normalize();
	hitPoint = Eigen::Matrix<T, 3, 1>::Constant(std::numeric_limits<T>::max());
	surfaceNormal = Eigen::Matrix<T, 3, 1>::Constant(0.0);
}

// makeRayBetweenPoints
template <typename T>
void Ray3<T>::resetToDefault()
{
	tMin = std::numeric_limits<T>::epsilon();
	tMax = std::numeric_limits<T>::max();
	hitBodyID = INVALID_ID;
	hitPolyID = INVALID_ID;
    distToHit = std::numeric_limits<T>::max();
	wasHit = false;

	bary0 = bary1 = bary2 = std::numeric_limits<T>::max();
	
	hitPoint = Eigen::Matrix<T, 3, 1>::Constant(std::numeric_limits<T>::max());
	surfaceNormal = Eigen::Matrix<T, 3, 1>::Constant(0.0);
	origin = Eigen::Matrix<T, 3, 1>::Constant(std::numeric_limits<T>::max());
	dir = Eigen::Matrix<T, 3, 1>::Constant(0.0);
}

// makeRayBetweenPoints
template <typename T>
T Ray3<T>::makeRayBetweenPoints (const Eigen::Matrix<T,3,1>  & point1,
								 const Eigen::Matrix<T,3,1>  & point2)
{	
	origin = point1;
	dir = point2 - origin;
    tMin = std::numeric_limits<T>::epsilon();
	tMax = dir.norm() - std::numeric_limits<T>::epsilon();
	
	return dir.normalize();
}



