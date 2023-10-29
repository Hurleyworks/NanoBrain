

using namespace Eigen;

CameraBody::CameraBody()
{
    pose.setIdentity();
}

CameraBody::~CameraBody()
{
}

void CameraBody::lookAt (const Eigen::Vector3f& eyePoint, const Eigen::Vector3f& target, const Eigen::Vector3f& up)
{
    pose.translation() = eyePoint;
    this->target = target;
    Eigen::Vector3f f = (target - eyePoint).normalized();

    eye = eyePoint;
    viewDirection = f;

   // mace::vecStr3f (f, DBUG, "View direction");

    Matrix3f camAxes;
    camAxes.col (2) = -f;
    camAxes.col (0) = worldUp.cross (camAxes.col (2)).normalized();
    camAxes.col (1) = camAxes.col (2).cross (camAxes.col (0)).normalized();
    orientation = Quaternionf (camAxes);

    forward = -(orientation * Vector3f::UnitZ());

    pose.linear() = -orientation.toRotationMatrix();

    viewMatrixCached = false;
}

void CameraBody::lookAt (const Eigen::Vector3f& eyePoint, const Eigen::Vector3f& target)
{
   
    this->target = target;
    eye = eyePoint;

    pose.translation() = eye;

    Eigen::Vector3f f = (target - eyePoint).normalized();
    viewDirection = f;

    /// Check for degeneracies.If the upDir and targetDir are parallel
    // or opposite, then compute a new, arbitrary up direction that is
    // not parallel or opposite to the targetDir.
    Vector3f upDir = worldUp;

    if (upDir.cross (f).squaredNorm() == 0)
    {
        upDir = f.cross (Vector3f::UnitX());
        if (upDir.squaredNorm() == 0)
            upDir = f.cross (Vector3f::UnitZ());

        upDir *= -1.0f; // to match Cinder
    }

    Matrix3f camAxes;
    camAxes.col (2) = -f;
    camAxes.col (0) = upDir.cross (camAxes.col (2)).normalized();
    camAxes.col (1) = camAxes.col (2).cross (camAxes.col (0)).normalized();
    orientation = Quaternionf (camAxes);

    forward = -(orientation * Vector3f::UnitZ());

    pose.linear() = orientation.toRotationMatrix();

    viewMatrixCached = false;
}

void CameraBody::lookAt (const Eigen::Vector3f& eyePoint)
{
    eye = eyePoint;

    pose.translation() = eye;

    Eigen::Vector3f f = (target - eyePoint).normalized();
    viewDirection = f;

    /// Check for degeneracies.If the upDir and targetDir are parallel
    // or opposite, then compute a new, arbitrary up direction that is
    // not parallel or opposite to the targetDir.
    Vector3f upDir = worldUp;

    if (upDir.cross (f).squaredNorm() == 0)
    {
        upDir = f.cross (Vector3f::UnitX());
        if (upDir.squaredNorm() == 0)
            upDir = f.cross (Vector3f::UnitZ());

        upDir *= -1.0f; // to match Cinder
    }

    Matrix3f camAxes;
    camAxes.col (2) = -f;
    camAxes.col (0) = upDir.cross (camAxes.col (2)).normalized();
    camAxes.col (1) = camAxes.col (2).cross (camAxes.col (0)).normalized();
    orientation = Quaternionf (camAxes);

    forward = -(orientation * Vector3f::UnitZ());

    pose.linear() = orientation.toRotationMatrix();

    viewMatrixCached = false;
}

void CameraBody::rotateAroundTarget (const Eigen::Quaternionf& q)
{
    // update the transform matrix
    if (!viewMatrixCached)
        calcViewMatrix();

    Vector3f t = viewMatrix * target;

    viewMatrix = Translation3f (t) * q * Translation3f (-t) * viewMatrix;

    Quaternionf qa (viewMatrix.linear());
    qa = qa.conjugate();
    orientation = qa;

    eye = -(qa * viewMatrix.translation());

    pose.translation() = eye;

    forward = -(orientation * Vector3f::UnitZ());
    viewDirection = (target - eye).normalized();

    pose.linear() = orientation.toRotationMatrix();

    viewMatrixCached = false;
}

void CameraBody::zoom (float d)
{
    float dist = (eye - target).norm();
    if (dist > d)
    {
        eye = eye + viewDirection * d;
        forward = -(orientation * Vector3f::UnitZ());
        viewMatrixCached = false;
        pose.translation() = eye;
    }
}

void CameraBody::track (const Eigen::Vector2f& point2D)
{
    Vector3f newPoint3D;
    bool newPointOk = mapToSphere (point2D, newPoint3D);

    if (lastPointOk && newPointOk)
    {
        Vector3f axis = lastPoint3D.cross (newPoint3D).normalized();
        float cos_angle = lastPoint3D.dot (newPoint3D);
        if (std::abs (cos_angle) < 1.0)
        {
            float angle = 2.0f * acos (cos_angle);
            //if (mMode == Around)
            rotateAroundTarget (Quaternionf (AngleAxisf (angle, axis)));
            //else
            //mpCamera->localRotate (Quaternionf (AngleAxisf (-angle, axis)));
        }
    }

    lastPoint3D = newPoint3D;
    lastPointOk = newPointOk;
}

void CameraBody::calcMatrices() const
{
    if (!viewMatrixCached) calcViewMatrix();
}

void CameraBody::calcViewMatrix() const
{
    mW = viewDirection.normalized();
    mU = orientation * Vector3f::UnitX();
    mV = orientation * Vector3f::UnitY();

    Quaternionf q = orientation.conjugate();
    viewMatrix.linear() = q.toRotationMatrix();

    if (!wabi::isOrthogonal<float> (viewMatrix.linear()))
    {
        Matrix3f m = viewMatrix.linear();
        if (!wabi::reOrthogonalize (m))
        {
            throw std::runtime_error ("Could not fix non-orthogongal matrix");
        }

        viewMatrix.linear() = m;
    }

    viewMatrix.translation() = -(viewMatrix.linear() * eye);

    forward = -(orientation * Vector3f::UnitZ());

    viewMatrixCached = true;
    inverseModelViewCached = false;
}

void CameraBody::calcInverseView() const
{
}

bool CameraBody::mapToSphere (const Eigen::Vector2f& p2, Eigen::Vector3f& v3)
{
    int w = sensor.getPixelResolution().x();
    int h = sensor.getPixelResolution().y();

    if ((p2.x() >= 0) && (p2.x() <= w &&
                          (p2.y() >= 0) && (p2.y() <= h)))
    {
        double x = (double)(p2.x() - 0.5 * w) / (double)w;
        double y = (double)(0.5 * h - p2.y()) / (double)h;
        double sinx = sin (M_PI * x * 0.5);
        double siny = sin (M_PI * y * 0.5);
        double sinx2siny2 = sinx * sinx + siny * siny;

        v3.x() = sinx;
        v3.y() = siny;
        v3.z() = sinx2siny2 < 1.0 ? sqrt (1.0 - sinx2siny2) : 0.0;

        return true;
    }
    else
        return false;
    ;
}
