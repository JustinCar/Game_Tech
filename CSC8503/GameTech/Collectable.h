#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "Player.h"
#include "../CSC8503Common/PositionConstraint.h"

namespace NCL {
	namespace CSC8503 {
		class Collectable : public GameObject
		{
		public:
			Collectable(int p, Vector3 pos);
			~Collectable();
			virtual void Update(float dt);
			virtual void Trigger(GameObject& obj);

			void SetConstraint(PositionConstraint* c) { constraint = c; };
			PositionConstraint* GetConstraint() { return constraint; };

			Vector3 GetOriginalPosition() { return originalPosition; };

			int GetPoints() { return points; };

		protected:

			PositionConstraint* constraint;

			Vector3 originalPosition;

			int points;
		};
	}
}

