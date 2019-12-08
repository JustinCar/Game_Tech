#pragma once
#include "Enemy.h"
namespace NCL {
	namespace CSC8503 {
		class Enemy;

		typedef void(*EnemyFunc)(Enemy& data, float dt);

		class EnemyChaseSuperState : public State {
		public:
			EnemyChaseSuperState(Enemy& e);

			~EnemyChaseSuperState();

			virtual void Update(float dt);
		protected:
			Enemy& enemy;
			StateMachine* machine;
			EnemyFunc Chase;
			EnemyFunc Attack;
		};
	}
}

