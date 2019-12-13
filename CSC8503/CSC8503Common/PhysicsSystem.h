#pragma once
#include "../CSC8503Common/GameWorld.h"
#include <set>
#include "../CSC8503Common/PositionConstraint.h"
#include "../GameTech/Player.h"

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}

			void SetGravity(const Vector3& g);
			void SetIsServer(bool b) { isServer = b; };
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			bool ShouldCollide(GameObject* a, GameObject* b); // Check layering
			void HandleCollision(GameObject* a, GameObject* b);
			void HandleCollectable(GameObject* player, GameObject* collectable);
			void HandleScoreIncrease(GameObject* player);

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();

			bool IsStatic(int layer);

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;

			GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;
			float	frameDT;

			bool isServer;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo>		broadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo>	broadphaseCollisionsVec;
			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;
		};
	}
}

