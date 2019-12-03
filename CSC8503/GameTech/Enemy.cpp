#include "Enemy.h"
using namespace NCL;
using namespace CSC8503;

Enemy::Enemy() : GameObject("ENEMY")
{
	speed = 20;
	grid = NavigationGrid("TestGrid2.txt");
	pathfindingOffSet = Vector3(-250, 10, -195);
	index = 0;
	currentDirection = Vector3(0, 0, 0);

}

Enemy::~Enemy()
{

}

void Enemy::UpdateEnemy(float dt)
{
	if (pathNodes.size() == 0)
	{
		GeneratePath();
		if (pathNodes.size() == 0)
			return;
	}
		

	Vector3 dir = transform.GetWorldPosition() - pathNodes[index];
	dir.Normalise();

	physicsObject->AddForce(dir * speed);

	if (Vector3::Distance(transform.GetWorldPosition(), pathNodes[index]) <= 1)
	{
		index++;

		if (index >= pathNodes.size())
		{
			index = 0;
			GeneratePath();
		}
			
	}
		
}

void Enemy::GeneratePath() 
{
	pathNodes.clear();
	NavigationPath outPath;

	Vector3 position = transform.GetWorldPosition();
	if (position == Vector3(0, 0, 0))
		return;

	position -= pathfindingOffSet;

	position.x = roundToNearestTen(position.x);
	position.z = roundToNearestTen(position.z);
	position.y = 0;
	// round position to nearest ten;

	int randX = rand() % 23 + 1;
	randX *= 10;

	int randZ = rand() % 20 + 1;
	randZ *= 10;
	Vector3 endPos(randX, 0, randZ);

	bool found = grid.FindPath(position, endPos, outPath);

	/*int i = 0;
	while (!found) 
	{
		int randX = rand() % 23 + 1;
		randX *= 10;

		int randZ = rand() % 20 + 1;
		randZ *= 10;
		Vector3 endPos(randX, 0, randZ);

		found = grid.FindPath(position, endPos, outPath);
	}*/

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathNodes.push_back(pos);
	}
}

//void Enemy::DisplayPathfinding() {
//	for (int i = 1; i < pathNodes.size(); ++i) {
//		Vector3 a = pathNodes[i - 1];
//
//		a += pathfindingOffSet;
//
//		Vector3 b = pathNodes[i];
//
//		b += pathfindingOffSet;
//
//		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
//	}
//}

int Enemy::roundToNearestTen(int num)
{
	// Already multiple of 10
	if (num % 10 == 0)
		return num;

	// Round down
	if ((num % 10) < 5)
		return num - num % 10;
	
	// Round up
	return (10 - num % 10) + num;
}