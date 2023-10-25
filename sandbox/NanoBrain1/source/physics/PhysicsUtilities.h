#pragma once

#include <mace_core/mace_core.h>
#include <ndNewton.h>

// state of the engine here so all can see
static const char* PhysicsEngineStateTable[] =
    {
        "Running",
        "Paused",
        "Reset",
        "Invalid"};

struct PhysicsEngineState
{
    enum EPhysicsEngineState
    {
        Running,
        Paused,
        Reset,
        Count,
        Invalid = Count
    };

    union
    {
        EPhysicsEngineState name;
        unsigned int value;
    };

    PhysicsEngineState (EPhysicsEngineState name) :
        name (name) {}
    PhysicsEngineState (unsigned int value) :
        value (value) {}
    PhysicsEngineState() :
        value (Invalid) {}
    operator EPhysicsEngineState() const { return name; }
    const char* toString() const { return PhysicsEngineStateTable[value]; }
};

static const char* BodyTypeTable[] =
    {
        "None",
        "Static",
        "Dynamic",
        "Fluid",
        "Invalid"};

struct BodyType
{
    enum EBodyType
    {
        None,
        Static,
        Dynamic,
        Fluid,
        Count,
        Invalid = Count
    };

    union
    {
        EBodyType name;
        unsigned int value;
    };

    BodyType (EBodyType name) :
        name (name) {}
    BodyType (unsigned int value) :
        value (value) {}
    BodyType() :
        value (Invalid) {}
    operator EBodyType() const { return name; }
    const char* toString() const { return BodyTypeTable[value]; }
    static BodyType FromString (const char* str) { return mace::TableLookup (str, BodyTypeTable, Count); }
};

static const char* CollisionShapeTable[] =
    {
        "Box",
        "Ball",
        "Capsule",
        "ConvexHull",
        "Mesh",
        "DynamicMesh",
        "Invalid"};

struct CollisionShape
{
    enum ECollisionShape
    {
        Box,
        Ball,
        Capsule,
        ConvexHull,
        DynamicMesh,
        Mesh,
        Count,
        Invalid = Count
    };

    union
    {
        ECollisionShape name;
        unsigned int value;
    };

    CollisionShape (ECollisionShape name) :
        name (name) {}
    CollisionShape (unsigned int value) :
        value (value) {}
    CollisionShape() :
        value (Invalid) {}
    operator ECollisionShape() const { return name; }
    const char* toString() const { return CollisionShapeTable[value]; }
    static CollisionShape FromString (const char* str) { return mace::TableLookup (str, CollisionShapeTable, Count); }
};

const BodyType DEFAULT_BODY_TYPE = BodyType::None;
const CollisionShape DEFAULT_COLLISION_SHAPE = CollisionShape::ConvexHull;
const float DEFAULT_DYNANMIC_MASS = 1.0f;
const float DEFAULT_ADHESION = 0.0f;
const float DEFAULT_STATIC_MASS = 0.0f;
const float DEFAULT_DYNAMIC_MASS = 2.0f;
const float DEFAULT_STATIC_FRICTION = 0.8f;
const float DEFAULT_DYNAMIC_FRICTION = 0.4f;
const float DEFAULT_BOUNCINESS = 0.0f;
const Eigen::Vector3d DEFAULT_FORCE = Eigen::Vector3d (0.0f, -10.0f, 0.0f);
const Eigen::Vector3d DEFAULT_VELOCITY = Eigen::Vector3d (0.0f, 0.0f, 0.0f);
const uint32_t DEFAULT_SLEEP_STATE = 0;

struct PhysicsDesc
{
    BodyType bodyType = DEFAULT_BODY_TYPE;
    CollisionShape shape = DEFAULT_COLLISION_SHAPE;
    float mass = DEFAULT_DYNANMIC_MASS;
    float adhesion = DEFAULT_ADHESION;
    float bounciness = DEFAULT_BOUNCINESS;
    float staticFriction = DEFAULT_STATIC_FRICTION;
    float dynamicFriction = DEFAULT_DYNAMIC_FRICTION;
    uint32_t sleepState = DEFAULT_SLEEP_STATE;
    Eigen::Vector3d force = DEFAULT_FORCE;
    Eigen::Vector3d velocity = DEFAULT_VELOCITY;

    void resetToDefault()
    {
        bodyType = DEFAULT_BODY_TYPE;
        shape = DEFAULT_COLLISION_SHAPE;
        mass = DEFAULT_DYNANMIC_MASS;
        sleepState = DEFAULT_SLEEP_STATE;
        adhesion = DEFAULT_ADHESION;
        staticFriction = DEFAULT_STATIC_FRICTION;
        dynamicFriction = DEFAULT_DYNAMIC_FRICTION;
        bounciness = DEFAULT_BOUNCINESS;
        force = DEFAULT_FORCE;
        velocity = DEFAULT_VELOCITY;
    }

    void debug()
    {
        LOG (DBUG) << "Body type: " << bodyType.toString();
        LOG (DBUG) << "CollisionShape: " << shape.toString();
        LOG (DBUG) << "Adhesion" << adhesion;
        LOG (DBUG) << "Mass: " << mass;
        LOG (DBUG) << "Static friction: " << staticFriction;
        LOG (DBUG) << "Dynamic friction: " << dynamicFriction;
        LOG (DBUG) << "Bounciness: " << bounciness;
        LOG (DBUG) << "Force: " << force.x() << ", " << force.y() << ", " << force.z();
        LOG (DBUG) << "Velocity: " << velocity.x() << ", " << velocity.y() << ", " << velocity.z();
    }
};

// Converts from Eigen Affine transforms to Newton matrices
inline bool eigenToNewton (const Eigen::Affine3f& transform, ndMatrix& pose)
{
    std::memcpy (&pose[0][0], transform.data(), 16 * sizeof (float));

    return true;
}