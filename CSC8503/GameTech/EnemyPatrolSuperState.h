#pragma once
#include "Enemy.h"
namespace NCL {
	namespace CSC8503 {
		class Enemy;

		typedef void(*EnemyFunc)(Enemy& data, float dt);

		class EnemyPatrolSuperState : public State {
		public:
			EnemyPatrolSuperState(Enemy& e);

			~EnemyPatrolSuperState();

			virtual void Update(float dt);
		protected:
			Enemy& enemy;
			StateMachine* machine;
			EnemyFunc Idle;
			EnemyFunc Patrol;
		};

		class EnemyState : public State {
		public:
			EnemyState(EnemyFunc someFunc, Enemy& someData) : funcData(someData) {
				func = someFunc;
			}

			virtual void Update(float dt) {
				func(funcData, dt);
			}
		protected:
			EnemyFunc func;
			Enemy& funcData;
		};
	}
}

