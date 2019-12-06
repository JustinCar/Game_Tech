#pragma once
#include "Enemy.h"
namespace NCL {
	namespace CSC8503 {
		class Enemy;
		class EnemyPatrolSuperState : public State {
		public:
			EnemyPatrolSuperState(Enemy* e);

			~EnemyPatrolSuperState();

			virtual void Update(float dt);
		protected:
			Enemy* enemy;
			StateMachine* machine;
			StateFunc Idle;
			StateFunc Patrol;
		};
	}
}

