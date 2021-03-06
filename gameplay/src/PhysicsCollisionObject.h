#ifndef PHYSICSCOLLISIONOBJECT_H_
#define PHYSICSCOLLISIONOBJECT_H_

#include "Vector3.h"
#include "PhysicsCollisionShape.h"
#include "PhysicsMotionState.h"

namespace gameplay
{

class Node;

/**
 * Base class for all gameplay physics objects that support collision events.
 */
class PhysicsCollisionObject
{
    friend class PhysicsController;
    friend class PhysicsConstraint;

public:

    /**
     * Represents the different types of collision objects.
     */
    enum Type
    {
        /**
         * PhysicsRigidBody type.
         */
        RIGID_BODY,

        /**
         * PhysicsCharacter type.
         */
        CHARACTER,

        /** 
         * PhysicsGhostObject type.
         */
        GHOST_OBJECT,

        /**
         * No collision object.
         */
        NONE
    };

    /** 
     * Defines a pair of rigid bodies that collided (or may collide).
     */
    class CollisionPair
    {
    public:

        /**
         * Constructor.
         */
        CollisionPair(PhysicsCollisionObject* objectA, PhysicsCollisionObject* objectB);

        /**
         * Less than operator (needed for use as a key in map).
         * 
         * @param collisionPair The collision pair to compare.
         * @return True if this pair is "less than" the given pair; false otherwise.
         */
        bool operator < (const CollisionPair& collisionPair) const;

        /**
         * The first object in the collision.
         */
        PhysicsCollisionObject* objectA;

        /**
         * The second object in the collision.
         */
        PhysicsCollisionObject* objectB;
    };

    /**
     * Collision listener interface.
     */
    class CollisionListener
    {
        friend class PhysicsCollisionObject;
        friend class PhysicsController;

    public:

        /**
         * The type of collision event.
         */
        enum EventType
        {
            /**
             * Event fired when the two rigid bodies start colliding.
             */
            COLLIDING,

            /**
             * Event fired when the two rigid bodies no longer collide.
             */
            NOT_COLLIDING
        };

        /**
         * Virtual destructor.
         */
        virtual ~CollisionListener() { }

        /**
         * Called when a collision occurs between two objects in the physics world.
         * 
         * @param type The type of collision event.
         * @param collisionPair The two collision objects involved in the collision.
         * @param contactPointA The contact point with the first object (in world space).
         * @param contactPointB The contact point with the second object (in world space).
         */
        virtual void collisionEvent(PhysicsCollisionObject::CollisionListener::EventType type,
                                    const PhysicsCollisionObject::CollisionPair& collisionPair,
                                    const Vector3& contactPointA = Vector3::zero(),
                                    const Vector3& contactPointB = Vector3::zero()) = 0;
    };

    /**
     * Virtual destructor.
     */
    virtual ~PhysicsCollisionObject();

    /**
     * Returns the type of the collision object.
     */
    virtual PhysicsCollisionObject::Type getType() const = 0;

    /**
     * Returns the type of the shape for this collision object.
     */
    PhysicsCollisionShape::Type getShapeType() const;

    /**
     * Returns the node associated with this collision object.
     */
    Node* getNode() const;

    /**
     * Returns the collision shape.
     *
     * @return The collision shape.
     */
    PhysicsCollisionShape* getCollisionShape() const;

    /**
     * Returns whether this collision object is kinematic.
     *
     * A kinematic collision object is an object that is not simulated by
     * the physics system and instead has its transform driven manually.
     *
     * @return Whether the collision object is kinematic.
     */
    bool isKinematic() const;

    /**
     * Returns whether this collision object is dynamic.
     *
     * A dynamic collision object is simulated entirely by the physics system,
     * such as with dynamic rigid bodies. 
     *
     * @return Whether the collision object is dynamic.
     */
    bool isDynamic() const;

    /**
     * Adds a collision listener for this collision object.
     * 
     * @param listener The listener to add.
     * @param object Optional collision object used to filter the collision event.
     */
    void addCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object = NULL);

    /**
     * Removes a collision listener.
     *
     * @param listener The listener to remove.
     * @param object Optional collision object used to filter the collision event.
     */
    void removeCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object = NULL);

    /**
     * Checks if this collision object collides with the given object.
     * 
     * @param object The collision object to test for collision with.
     * 
     * @return True if this object collides with the specified one; false otherwise.
     */
    bool collidesWith(PhysicsCollisionObject* object) const;

protected:

    /**
     * Constructor.
     */
    PhysicsCollisionObject(Node* node);

    /**
     * Returns the Bullet Physics collision object.
     *
     * @return The Bullet collision object.
     */
    virtual btCollisionObject* getCollisionObject() const = 0;

    /**
     * Returns the physics motion state.
     *
     * @return The motion state object.
     */
    PhysicsMotionState* getMotionState() const;

    // Common member variables
    /**
     * Pointer to Node contained by this collision object.
     */ 
    Node* _node;

    /** 
     * The PhysicsCollisionObject's motion state.
     */
    PhysicsMotionState* _motionState;
    
    /**
     * The PhysicsCollisionObject's collision shape.
     */
    PhysicsCollisionShape* _collisionShape;

};

}

#endif
