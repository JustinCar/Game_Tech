#include "EnemyPatrolSuperState.h"
using namespace NCL;
using namespace CSC8503;

EnemyPatrolSuperState::EnemyPatrolSuperState(Enemy* e) {
	enemy = e;
	machine = new StateMachine();

	StateFunc Idle = [](void* data, float dt) {
		Enemy* obj = (Enemy*)data;
		obj->Patrol(dt);
	};

	StateFunc Patrol = [](void* data, float dt) {
		Enemy* obj = (Enemy*)data;
		obj->Patrol(dt);
	};

	GenericState* stateA = new GenericState(Idle, (void*)& enemy);
	GenericState* stateB = new GenericState(Patrol, (void*)& enemy);
	machine->AddState(stateA);
	machine->AddState(stateB);

	float a = enemy->getTimeSpentIdle();
	float b = enemy->getTimeToSpendIdle();

	GenericTransition <float&, float >* transitionA =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::GreaterThanTransition,
			enemy->getTimeSpentIdle(), enemy->getTimeToSpendIdle(), stateA, stateB);

	GenericTransition <float&, float >* transitionB =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::GreaterThanTransition,
			enemy->getTimeSpentPatrolling(), enemy->getTimeToSpendPatrolling(), stateB, stateA);

	machine->AddTransition(transitionA);
	machine->AddTransition(transitionB);
}

EnemyPatrolSuperState::~EnemyPatrolSuperState()
{
	delete machine;
}

void EnemyPatrolSuperState::Update(float dt) {
	machine->Update(dt);
}