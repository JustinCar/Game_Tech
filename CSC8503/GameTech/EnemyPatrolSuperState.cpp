#include "EnemyPatrolSuperState.h"
using namespace NCL;
using namespace CSC8503;

EnemyPatrolSuperState::EnemyPatrolSuperState(Enemy& e) : enemy(e) {
	machine = new StateMachine();

	Idle = [](Enemy& data, float dt) {
		Enemy& obj = (Enemy&)data;
		//std::cout << "IDLE!" << std::endl;
		obj.Idle(dt);
	};

	Patrol = [](Enemy& data, float dt) {
		Enemy& obj = (Enemy&)data;
		//std::cout << "PATROLLING!" << std::endl;
		obj.Patrol(dt);
	};

	EnemyState* stateA = new EnemyState(Idle, enemy);
	EnemyState* stateB = new EnemyState(Patrol, enemy);
	machine->AddState(stateA);
	machine->AddState(stateB);

	GenericTransition <float&, float>* transitionA =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::GreaterThanTransition,
			enemy.getTimeSpentIdle(), enemy.getTimeToSpendIdle(), stateA, stateB);

	GenericTransition <float&, float>* transitionB =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::GreaterThanTransition,
			enemy.getTimeSpentPatrolling(), enemy.getTimeToSpendPatrolling(), stateB, stateA);

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