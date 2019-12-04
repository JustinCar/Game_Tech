#include "Enemy.h"
using namespace NCL;
using namespace CSC8503;

Enemy::Enemy() : GameObject("ENEMY")
{
	speed = 80;
	//grid = NavigationGrid("TestGrid2.txt");

	grid = new NavigationGrid("TestGrid2.txt");
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

	DisplayPathfinding();

	Vector3 node = pathNodes[index];

	node += pathfindingOffSet;
		
	Vector3 dir = node - transform.GetWorldPosition();
	dir.Normalise();

	physicsObject->AddForce(dir * speed);

	if (Vector3::Distance(transform.GetWorldPosition(), node) <= 1)
	{
		index++;

		if (index >= pathNodes.size())
		{
			pathNodes.clear();
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

	position.x = roundToNearestTen(position.x);
	position.z = roundToNearestTen(position.z);
	position.y = 0;

	Vector3 newPos = position;
	newPos.y += 30;

	Debug::DrawLine(position, newPos, Vector4(0, 0, 1, 1));

	position -= pathfindingOffSet;
	
	// round position to nearest ten;

	int randX = rand() % 23 + 1;
	randX *= 10;

	int randZ = rand() % 20 + 1;
	randZ *= 10;
	Vector3 endPos(randX, 0, randZ);

	Vector3 newEndPos = endPos;
	newEndPos.y += 30;

	bool found = grid->FindPath(position, endPos, outPath);

	if (!found) 
	{
		int i = 0;
	}
	
	/*while (!found) 
	{
		int randX = rand() % 23 + 1;
		randX *= 10;

		int randZ = rand() % 20 + 1;
		randZ *= 10;
		Vector3 endPos(randX, 0, randZ);

		found = grid->FindPath(position, endPos, outPath);
	}*/

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathNodes.push_back(pos);
		int i = 0;
	}
}

void Enemy::DisplayPathfinding() {
	for (int i = 1; i < pathNodes.size(); ++i) {
		Vector3 a = pathNodes[i - 1];

		a += pathfindingOffSet;

		Vector3 b = pathNodes[i];

		b += pathfindingOffSet;

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

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