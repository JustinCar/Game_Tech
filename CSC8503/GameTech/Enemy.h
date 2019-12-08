#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\CSC8503Common\Debug.h"
#include "../../Common/Window.h"
#include "../../Common/Camera.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "EnemyPatrolSuperState.h"
#include "EnemyChaseSuperState.h"
#include <cmath>
#define PI 3.14159265
namespace NCL {
	namespace CSC8503 {

		class Enemy : public GameObject
		{
		public:
			Enemy(Vector3 position);
			~Enemy();
			void UpdateEnemy(float dt);
			void Patrol(float dt);
			void Idle(float dt);
			void Chase(float dt);
			void Attack(float dt);

			void setTimeSpentIdle(float t) { timeSpentIdle = t; };
			float& getTimeSpentIdle() { return timeSpentIdle; };

			void setTimeSpentPatrolling(float t) { timeSpentPatrolling = t; };
			float& getTimeSpentPatrolling() { return timeSpentPatrolling; };

			float getTimeToSpendIdle() { return timeToSpendIdle; };
			float getTimeToSpendPatrolling() { return timeToSpendPatrolling; };

			float& getDistanceFromPlayer() { return distanceFromPlayer; };
			float getChaseRadius() { return chaseRadius; };
			float getAttackRadius() { return attackRadius; };

			void setPlayer(GameObject* p) { player = p; };
			

		protected:
			void GeneratePath();
			int roundToNearestTen(int num);
			void DisplayPathfinding();

			void InitStateMachine();

			float speed; 
			float chaseSpeed;
			float rotationSpeed;

			StateMachine* testMachine;

			float timeSpentIdle;
			float timeSpentPatrolling;

			float timeToSpendIdle;
			float timeToSpendPatrolling;

			float distanceFromPlayer;
			float chaseRadius;
			float attackRadius;

			GameObject* player;

			StateMachine* stateMachine;

			vector<Vector3> pathNodes;
			int index;
			NavigationGrid* grid;
			Vector3 pathfindingOffSet;
			Vector3 currentDirection;
		};
	}
}
