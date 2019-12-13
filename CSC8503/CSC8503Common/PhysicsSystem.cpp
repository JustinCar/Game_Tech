#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "../../Common/Quaternion.h"

#include "Constraint.h"

#include "Debug.h"

#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : gameWorld(g)	{
	applyGravity	= true;
	useBroadPhase	= true;	
	dTOffset		= 0.0f;
	globalDamping	= 0.95f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
}

PhysicsSystem::~PhysicsSystem()	{

}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/
void PhysicsSystem::Update(float dt) {
	GameTimer testTimer;
	testTimer.GetTimeDeltaSeconds();

	frameDT = dt;

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	float iterationDt = 1.0f / 120.0f; //Ideally we'll have 120 physics updates a second 

	if (dTOffset > 8 * iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 15.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting physics iterations to 15" << iterationDt << std::endl;
	}
	else if (dTOffset > 4  * iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 30.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting iteration dt to 4 case " << iterationDt << std::endl;
	}
	else if (dTOffset > 2* iterationDt) { //the physics engine cant catch up!
		iterationDt = 1.0f / 60.0f; //it'll just have to run bigger timesteps...
		//std::cout << "Setting iteration dt to 2 case " << iterationDt << std::endl;
	}
	else {
		//std::cout << "Running normal update " << iterationDt << std::endl;
	}

	int constraintIterationCount = 10;
	iterationDt = dt;

	if (useBroadPhase) {
		UpdateObjectAABBs();
	}

	while(dTOffset > iterationDt * 0.5) {
		IntegrateAccel(iterationDt); //Update accelerations from external forces


		if (useBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = iterationDt /  (float)constraintIterationCount;

		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);	
		}

		IntegrateVelocity(iterationDt); //update positions from new velocity changes

		dTOffset -= iterationDt; 
	}
	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	//std::cout << iteratorCount << " , " << iterationDt << std::endl;
	float time = testTimer.GetTimeDeltaSeconds();
	//std::cout << "Physics time taken: " << time << std::endl;
}

void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);

			HandleCollision(i->a, i->b);
		}
		(*i).framesLeft = (*i).framesLeft - 1;
		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateBroadphaseAABB();
	}
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset 
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	
	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;

		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;

			}

			CollisionDetection::CollisionInfo info;

			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}
		}
	}
}

void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject * physB = b.GetPhysicsObject();
	
	Transform & transformA = a.GetTransform();
	Transform & transformB = b.GetTransform();
	
	float totalMass = physA -> GetInverseMass() + physB -> GetInverseMass();
	
	// Separate them out using projection
	transformA.SetWorldPosition(transformA.GetWorldPosition() -
		(p.normal * p.penetration * (physA -> GetInverseMass() / totalMass)));
	
	transformB.SetWorldPosition(transformB.GetWorldPosition() +
		(p.normal * p.penetration * (physB -> GetInverseMass() / totalMass)));
	Vector3 relativeA = p.position - transformA.GetWorldPosition();
	Vector3 relativeB = p.position - transformB.GetWorldPosition();
	
	Vector3 angVelocityA =
	Vector3::Cross(physA -> GetAngularVelocity(), relativeA);
	Vector3 angVelocityB =
	Vector3::Cross(physB -> GetAngularVelocity(), relativeB);
	
	Vector3 fullVelocityA = physA -> GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB -> GetLinearVelocity() + angVelocityB;
	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);
	
	// now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA -> GetInertiaTensor() *
		Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB -> GetInertiaTensor() *
		Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);
	
	float cRestitution = 0.66f; // disperse some kinectic energy
	
	float j = (-(1.0f + cRestitution) * impulseForce) /
		(totalMass + angularEffect);
	
	Vector3 fullImpulse = p.normal * j;
	physA -> ApplyLinearImpulse(-fullImpulse);
	physB -> ApplyLinearImpulse(fullImpulse);
	
	physA -> ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB -> ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));

}

void PhysicsSystem::HandleCollision(GameObject* a, GameObject* b)
{
	// Pickup collectables
	if (a->getLayer() == 4 && b->getLayer() == 2)
		HandleCollectable(b, a);

	if (b->getLayer() == 4 && a->getLayer() == 2)
		HandleCollectable(a, b);

	// Returned to island
	if (b->getLayer() == 5 && a->getLayer() == 2)
		HandleScoreIncrease(a);

	if (a->getLayer() == 5 && b->getLayer() == 2)
		HandleScoreIncrease(b);

	// Returned to island
	if (b->getLayer() == 5 && a->getLayer() == 2)
		HandleScoreIncrease(a);
}

void PhysicsSystem::HandleScoreIncrease(GameObject* player)
{
	Player* p = static_cast<Player*>(&(*player));

	if (p->getCollectables().size() == 0)
		return;

	for (int i = 0; i < p->getCollectables().size(); i++)
	{
		Collectable* c = static_cast<Collectable*>(&(*p->getCollectables().front()));

		int points = c->GetPoints();

		if (!gameWorld.GetIsNetworkedGame())
			gameWorld.increaseScore(points);
		else 
		{
			p->GetNetworkObject()->increaseScore(points);

			if (p->GetNetworkObject()->GetID() == 1000)
			{
				gameWorld.increasePlayerOneScore(points);
				gameWorld.IncreasePlayerOneTotal(points);
			}
			else
			{
				gameWorld.increasePlayerTwoScore(points);
				gameWorld.IncreasePlayerTwoTotal(points);
			}	
		}

		gameWorld.RemoveConstraint(c->GetConstraint());

		if (!gameWorld.GetIsNetworkedGame())
			gameWorld.RemoveGameObject(p->getCollectables().front());
		else
			c->GetTransform().SetWorldPosition(Vector3(-1000, -1000, -1000));

		p->getCollectables().pop();

		gameWorld.DecrementCollectableCount();
	}
	int i = 0;
}

void PhysicsSystem::HandleCollectable(GameObject* player, GameObject* collectable)
{
	Player* p = static_cast<Player*>(&(*player));
	PositionConstraint* constraint;

	float maxDistance = 2;

	if (p->getCollectables().size() == 0)
		constraint = new PositionConstraint(player, collectable, maxDistance);
	else
		constraint = new PositionConstraint(p->getCollectables().back(), collectable, maxDistance);
		
	if (p->GetNetworkObject()->GetID() == 1000)
		collectable->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	else 
		collectable->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	Collectable* c = static_cast<Collectable*>(&(*collectable));
	c->SetConstraint(constraint);

	collectable->Trigger(*player);
	gameWorld.AddConstraint(constraint);
}



bool PhysicsSystem::ShouldCollide(GameObject* a, GameObject* b)
{
	if (!a->getLayer() || !b->getLayer())
		return true;

	if (a->getLayerMask() & (1 << (b->getLayer() - 1)))
		return false;

	if (b->getLayerMask() & (1 << (a->getLayer() - 1)))
		return false;

	return true;
}

void PhysicsSystem::BroadPhase() {
	broadphaseCollisions.clear();
	QuadTree < GameObject* > tree(Vector2(1024, 1024), 7, 6);
	
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i) -> GetBroadphaseAABB(halfSizes)) {
			continue;
			
		}
		Vector3 pos = (*i) -> GetConstTransform().GetWorldPosition();
		tree.Insert(*i, pos, halfSizes);
	}
	tree.OperateOnContents([&](std::list < QuadTreeEntry < GameObject* > >& data) {
		CollisionDetection::CollisionInfo info;
		
		for (auto i = data.begin(); i != data.end(); ++i) {
			for (auto j = std::next(i); j != data.end(); ++j) {
				// is this pair of items already in the collision set -
				// if the same pair is in another quadtree node together etc

				if (!ShouldCollide((*i).object, (*j).object))
					continue;
					
				info.a = min((*i).object, (*j).object);
				info.b = max((*i).object, (*j).object);
				broadphaseCollisions.insert(info);
			}
			
		}
	});

}

void PhysicsSystem::NarrowPhase() {
	for (std::set < CollisionDetection::CollisionInfo >::iterator
		i = broadphaseCollisions.begin();
		i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;

		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info); // insert into our main set
			
		}
	}
}

bool PhysicsSystem::IsStatic(int layer)
{
	if (layer == 1 ||
		layer == 5 ||
		layer == 6)
		return true;

	return false;
}

void PhysicsSystem::IntegrateAccel(float dt) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	
	for (auto i = first; i != last; ++i) {
		PhysicsObject * object = (*i) -> GetPhysicsObject();
		if (object == nullptr) {
			continue; // No physics object for this GameObject !
			
		}

		if ((*i)->getLayer() == 2)
			int i = 0;

		float inverseMass = object -> GetInverseMass();
		
		Vector3 linearVel = object -> GetLinearVelocity();
		Vector3 force = object -> GetForce();
		Vector3 accel = force * inverseMass;
		
		if (applyGravity && inverseMass > 0) {
			accel += gravity; 
		}
		
		linearVel += accel * dt; // integrate accel !
		object -> SetLinearVelocity(linearVel);

		// Angular stuff
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor(); // update tensor vs orientation

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * dt; // integrate angular accel !
		object->SetAngularVelocity(angVel);

	}

}

void PhysicsSystem::IntegrateVelocity(float dt) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	float dampingFactor = 1.0f - 0.95f;
	float frameDamping = powf(dampingFactor, dt);
	
	for (auto i = first; i != last; ++i) {
		PhysicsObject * object = (*i) -> GetPhysicsObject();
		if (object == nullptr) {
			continue;

		}
		Transform & transform = (*i) -> GetTransform();
		
		Vector3 position = transform.GetLocalPosition();
		Vector3 linearVel = object -> GetLinearVelocity();
		position += linearVel * dt;
		transform.SetLocalPosition(position);
		
		linearVel = linearVel * frameDamping;
		object -> SetLinearVelocity(linearVel);

		
		Quaternion orientation = transform.GetLocalOrientation();
		Vector3 angVel = object -> GetAngularVelocity();
		
		orientation = orientation +
			(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();
		
		transform.SetLocalOrientation(orientation);
		
		
		angVel = angVel * frameDamping;
		object -> SetAngularVelocity(angVel);

	}

}

void PhysicsSystem::ClearForces() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		
		(*i)->GetPhysicsObject()->ClearForces();
	}
}


void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}