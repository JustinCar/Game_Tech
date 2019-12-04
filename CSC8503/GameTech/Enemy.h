#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "..\CSC8503Common\Debug.h"
#include "../../Common/Window.h"
#include "../../Common/Camera.h"
#include "../CSC8503Common/NavigationGrid.h"
#include <cmath>
#define PI 3.14159265
namespace NCL {
	namespace CSC8503 {
		class Enemy : public GameObject
		{
		public:
			Enemy();
			~Enemy();
			void UpdateEnemy(float dt);

			

		protected:
			void GeneratePath();
			int roundToNearestTen(int num);
			void DisplayPathfinding();

			float speed; 
			vector<Vector3> pathNodes;
			int index;
			NavigationGrid* grid;
			Vector3 pathfindingOffSet;
			Vector3 currentDirection;
		};
	}
}
