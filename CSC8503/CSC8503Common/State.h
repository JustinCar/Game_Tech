#pragma once
#include "StateMachine.h"
namespace NCL {
	namespace CSC8503 {
		class State		{
		public:
			State() {}
			virtual ~State() {}
			virtual void Update(float dt) = 0; //Pure virtual base class
		};

		typedef void(*StateFunc)(void*, float dt);

		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* someData) {
				func		= someFunc;
				funcData	= someData;
			}
			virtual void Update(float dt) {
				if (funcData != nullptr) {
					func(funcData, dt);
				}
			}
		protected:
			StateFunc func;
			void* funcData;
		};

		/*class EnemyState : public State {
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
		};*/

	}
}