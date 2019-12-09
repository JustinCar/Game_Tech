#include "EnemyChaseSuperState.h"
using namespace NCL;
using namespace CSC8503;

EnemyChaseSuperState::EnemyChaseSuperState(Enemy& e) : enemy(e) {
	machine = new StateMachine();

	Chase = [](Enemy& data, float dt) {
		Enemy& obj = (Enemy&)data;
		//std::cout << "CHASING!" << std::endl;
		obj.Chase(dt);
	};

	Attack = [](Enemy& data, float dt) {
		Enemy& obj = (Enemy&)data;
		//std::cout << "ATTACKING!" << std::endl;
		obj.Attack(dt);
	};

	EnemyState* stateA = new EnemyState(Chase, enemy);
	EnemyState* stateB = new EnemyState(Attack, enemy);
	machine->AddState(stateA);
	machine->AddState(stateB);

	GenericTransition <float&, float>* transitionA =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::LessThanTransition,
			enemy.getDistanceFromPlayer(), enemy.getAttackRadius(), stateA, stateB);

	GenericTransition <float&, float>* transitionB =
		new GenericTransition <float&, float>(
			GenericTransition <float&, float>::GreaterThanTransition,
			enemy.getDistanceFromPlayer(), enemy.getAttackRadius(), stateB, stateA);

	machine->AddTransition(transitionA);
	machine->AddTransition(transitionB);
}

EnemyChaseSuperState::~EnemyChaseSuperState()
{
	delete machine;
}

void EnemyChaseSuperState::Update(float dt) {
	machine->Update(dt);
}