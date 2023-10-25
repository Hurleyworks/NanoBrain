

#include "NewtonOpsHandler.h"

constexpr float MIN_TRI_AREA = 1.0e-6f;

// ctor
NewtonOpsHandler::NewtonOpsHandler (PhysicsContextPtr ctx) :
    ctx (ctx)
{
}

// dtor
NewtonOpsHandler::~NewtonOpsHandler()
{
}

void NewtonOpsHandler::createConvexHullMesh (OptiXWeakNode weakNode)
{
}

ndShapeInstance NewtonOpsHandler::createCollisionShape (OptiXWeakNode weakNode)
{
    if (weakNode.expired()) return ndShapeInstance (nullptr);

    OptiXNode node = weakNode.lock();

    Vector3f sizes = node->st.modelBound.sizes();

    // find largest dimension for the radius of the sphere shape
    float maxDim = sizes.maxCoeff();

    ndShape* shape = nullptr;

    switch (node->desc.shape)
    {
        case CollisionShape::Ball:
            shape = new ndShapeSphere (maxDim * 0.5f);
            break;

        case CollisionShape::Box:
            shape = new ndShapeBox (sizes.x(), sizes.y(), sizes.z());
            break;

        case CollisionShape::ConvexHull:
        {
            MatrixXf V;
            node->g->extractVertexPositions (V);

            shape = new ndShapeConvexHull (V.cols(), 3 * sizeof (float), 0.0f, V.data(), 32);
            break;
        }

        case CollisionShape::Mesh:
        {
            MatrixXf V;
            node->g->extractVertexPositions (V);

            MatrixXu F;
            node->g->extractTriangleIndices (F);

            ndPolygonSoupBuilder meshBuilder;
            meshBuilder.Begin();

            uint32_t materialIndex = 0;
            for (int i = 0; i < F.cols(); i++)
            {
                const Vector3u& tri = F.col (i);

                // find triangle vertices
                Vector3f p0 = V.col (tri.x());
                Vector3f p1 = V.col (tri.y());
                Vector3f p2 = V.col (tri.z());

                // We can compute the triangle area using the cross product to determine the area
                // of the parallelogram and then halve it to get the area of the triangle.
                // Taking the cross product of these edges (edge1.cross (edge2)) gives us a vector,
                // and the magnitude of this vector represents the area of the parallelogram spanned by edge1 and edge2.
                Eigen::Vector3f edge1 = p1 - p0;
                Eigen::Vector3f edge2 = p2 - p0;
                float triangleArea = 0.5f * edge1.cross (edge2).norm();

                if (triangleArea < MIN_TRI_AREA)
                {
                    LOG (DBUG) << "Rejecting triangle area that is too small for Newton " << triangleArea;
                    continue;
                }

                ndVector face[3];
                face[0] = ndVector (p0[0], p0[1], p0[2], 0.0f);
                face[1] = ndVector (p1[0], p1[1], p1[2], 0.0f);
                face[2] = ndVector (p2[0], p2[1], p2[2], 0.0f);

                meshBuilder.AddFace (&face[0].m_x, sizeof (ndVector), 3, materialIndex);
            }

            bool optimized = true;

            meshBuilder.End (optimized);
            shape = new ndShapeStatic_bvh (meshBuilder);
            break;
        }

        default:
            shape = new ndShapeBox (sizes.x(), sizes.y(), sizes.z());
            break;
    }

    return ndShapeInstance (shape);
}
