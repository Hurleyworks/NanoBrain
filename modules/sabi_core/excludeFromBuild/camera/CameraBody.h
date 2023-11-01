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

using CameraHandle = std::shared_ptr<class CameraBody>;

class CameraBody 
{
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/3d-viewing-pinhole-camera/how-pinhole-camera-works-part-2
    // It is very important to remember that two parameters determine the angle of view : the focal length and the film size.
    // The angle of view changes when you change either one of these two parameters : the focal length or the film size.

    // For a fixed film size, changing the focal length will change the angle of view.
    // The longer the focal length, the narrower than angle of view. For a fixed focal length,
    // changing the film size will change the angle of view. The larger the film, the wider
    // the angle of view. If you wish to change the film size but keep the same angle of view,
    // you will need to adjust the focal length accordingly.

 public:
    CameraBody();
    ~CameraBody();


    float getFocalLength() const { return focalLength; }
    void setFocalLength (float length)
    {
        // sensor size is fixed at 3:2 on 35 mm:  0.036 x 0.024
        // compute the new vertical FOV from the incoming focal length ( tan = opp/adj )
        focalLength = length;
        float sensorHalfHeight = sensor.getSensorSize().y() / 2.0f;
        verticalFOVradians = 2 * std::tan (sensorHalfHeight / focalLength);
    }

    float getApeture() const { return apeture; }
    void setApeture (float apeture) { this->apeture = apeture; }

    void lookAt (const Eigen::Vector3f& eyePoint, const Eigen::Vector3f& target, const Eigen::Vector3f& up);
    void lookAt (const Eigen::Vector3f& eyePoint, const Eigen::Vector3f& target);
    void lookAt (const Eigen::Vector3f& eyePoint);

    wabi::Ray3f generateRay (uint32_t x, uint32_t y, bool transformed = true)
    {
        // FIXME aspect < 1.0f ?????

        float u = (2 * ((x + 0.5) * sensor.pixelSize().x()) - 1) * verticalFOVradians * sensor.getPixelAspectRatio();
        float v = (1 - 2 * ((y + 0.5) * sensor.pixelSize().y())) * verticalFOVradians;

        Eigen::Vector3f dir = Eigen::Vector3f (u, v, 1.0f).normalized();

        return wabi::Ray3<float> (pose.translation(), transformed ? pose.linear() * dir : dir);
    }

    // with jittering
    wabi::Ray3f generateRay (uint32_t x, uint32_t y, float jitterU, float jitterV, bool transformed = true)
    {
        // FIXME aspect < 1.0f ?????

        float u = (2 * ((x + jitterU) * sensor.pixelSize().x()) - 1) * verticalFOVradians * sensor.getPixelAspectRatio();
        float v = (1 - 2 * ((y + jitterV) * sensor.pixelSize().y())) * verticalFOVradians;

        Eigen::Vector3f dir = Eigen::Vector3f (u, v, 1.0f).normalized();

        return wabi::Ray3<float> (pose.translation(), transformed ? pose.linear() * dir : dir);
    }

    void rotateAroundTarget (const Eigen::Quaternionf& q);
    void zoom (float d);

    // trackball
    void startTracking() { lastPointOk = false; }
    void track (const Eigen::Vector2f& point2D);

    OIIO::ImageBuf& getSensorPixels() { return sensor.getImage(); }
    CameraSensor* getSensor() { return &sensor; }

	void setPose (const Pose& pose) { this->pose = pose; }
    const Pose& getPose() const { return pose; }
    Pose& getPose() { return pose; }

    bool isDirty() const { return dirty; }
    void setDirty (bool state) { dirty = state; }

    float getVerticalFOVradians() { return verticalFOVradians; }

    const Eigen::Matrix4f& getViewMatrix() const
    {
        if (!viewMatrixCached) 
            calcViewMatrix();
        return viewMatrix.matrix();
    }

    void setEyePoint (const Eigen::Vector3f& eyePoint, bool update = false)
    {
        eye = eyePoint;
    }

    void setTarget (const Eigen::Vector3f& newTarget)
    {
        target = newTarget;
    }

    Eigen::Vector3f& getEyePoint() { return eye; }
    const Eigen::Vector3f& getEyePoint() const { return eye; }
    const Eigen::Vector3f& getWorldUp() const { return worldUp; }
    const Eigen::Vector3f& getUp() const { return mV; }
    const Eigen::Vector3f& getRight() const { return mU; }
    const Eigen::Vector3f& getFoward() const { return forward; }
    const Eigen::Vector3f& getTarget() const { return target; }
    const Eigen::Vector3f& getViewDirection() const { return viewDirection; }
    const Eigen::Quaternionf& getOrientation() const { return orientation; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

 private:
    CameraSensor sensor;
    std::string name = "Perspective Camera";
    Pose pose;
    bool dirty = true;
    float verticalFOVradians = DEFAULT_FOV_DEGREES;
    float focalLength = 0.055f; // 55 mm lens
    float apeture = 0.0f;
    Eigen::Vector3f target = Eigen::Vector3f::Zero();

    // trackball
    Eigen::Vector3f lastPoint3D = Eigen::Vector3f::Zero();
    bool lastPointOk = false;
    bool mapToSphere (const Eigen::Vector2f& p2, Eigen::Vector3f& v3);

    Eigen::Vector3f getPosition() { return pose.translation(); }

    Eigen::Vector3f eye = DEFAULT_CAMERA_POSIIION;
    Eigen::Vector3f viewDirection = Eigen::Vector3f::UnitZ();
    mutable Eigen::Vector3f forward = Eigen::Vector3f::UnitX();
    Eigen::Quaternionf orientation;
    Eigen::Vector3f worldUp = Eigen::Vector3f::UnitY();

    mutable Eigen::Vector3f mU; // Right vector
    mutable Eigen::Vector3f mV; // Readjust up-vector
    mutable Eigen::Vector3f mW; // Negative view direction

    mutable Eigen::Affine3f viewMatrix;
    mutable bool viewMatrixCached;
    mutable Matrix4f inverseModelViewMatrix;
    mutable bool inverseModelViewCached;

    void calcMatrices() const;
    virtual void calcViewMatrix() const;
    virtual void calcInverseView() const;
};
