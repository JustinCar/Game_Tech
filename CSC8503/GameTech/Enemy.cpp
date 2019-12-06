#include "Enemy.h"

using namespace NCL;
using namespace CSC8503;

Enemy::Enemy(Vector3 position) : GameObject("ENEMY")
{
	speed = 80;
	rotationSpeed = 5;
	player = nullptr;
	grid = new NavigationGrid("TestGrid3.txt");

	position.x = roundToNearestTen(position.x);
	position.z = roundToNearestTen(position.z);

	layer = 3;
	layerMask = 5;

	while (!grid->ValidStartingPosition(position))
	{
		int xPos = rand() % 480;
		int zPos = rand() % 420;
		position = Vector3(xPos, 12, zPos);
	}

	transform.SetWorldPosition(position);

	timeToSpendIdle = 5.0f;
	timeToSpendPatrolling = 10.0f;

	timeSpentIdle = 0.0f;
	timeSpentPatrolling = 0.0f;

	distanceFromPlayer = 0.0f;
	chaseRadius = 5.0f;

	pathfindingOffSet = Vector3(0, 10, 0);
	index = 0;
	currentDirection = Vector3(0, 0, 0);

	InitStateMachine();
}

Enemy::~Enemy()
{

}

void Enemy::UpdateEnemy(float dt)
{
	if (player)
		distanceFromPlayer = Vector3::Distance(transform.GetWorldPosition(), player->GetTransform().GetWorldPosition());

	stateMachine->Update(dt);
}

void Enemy::InitStateMachine()
{
	stateMachine = new StateMachine();

	// Hierarchical State Machine
	StateFunc PatrolSuperState = [](void* data, float dt) {
		EnemyPatrolSuperState* realData = (EnemyPatrolSuperState*)data;
		realData->Update(dt);
		std::cout << "PATROLLING!" << std::endl;
	};

	StateFunc ChaseSuperState = [](void* data, float dt) {
		EnemyPatrolSuperState* realData = (EnemyPatrolSuperState*)data;
		realData->Update(dt);
		std::cout << "IDLE!" << std::endl;
	};

	EnemyPatrolSuperState* superState =  new EnemyPatrolSuperState(this);

	GenericState* stateA = new GenericState(PatrolSuperState, (void*) superState);
	GenericState* stateB = new GenericState(ChaseSuperState, (void*) superState);
	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	GenericTransition <float&, float >* transitionA =
		new GenericTransition <float&, float >(
			GenericTransition <float&, float >::LessThanTransition,
			distanceFromPlayer, chaseRadius, stateA, stateB);

	GenericTransition <float&, float >* transitionB =
		new GenericTransition <float&, float >(
			GenericTransition <float&, float >::GreaterThanTransition,
			distanceFromPlayer, chaseRadius, stateB, stateA);

	stateMachine->AddTransition(transitionA);
	stateMachine->AddTransition(transitionB);
}

void Enemy::Idle(float dt)
{
	timeSpentIdle += dt;
}

void Enemy::Patrol(float dt)
{
	if (pathNodes.size() == 0)
	{
		GeneratePath();
		if (pathNodes.size() == 0)
			return;
	}

	timeSpentPatrolling += dt;

	//DisplayPathfinding();

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
	NavigationPath outPath;

	Vector3 position = transform.GetWorldPosition();
	if (position == Vector3(0, 0, 0))
		return;

	position.x = roundToNearestTen(position.x);
	position.z = roundToNearestTen(position.z);
	position.y = 0;
	
	//round position to nearest ten;

	int randX = rand() % 47 + 1;
	randX *= 10;

	int randZ = rand() % 41 + 1;
	randZ *= 10;
	Vector3 endPos(randX, 0, randZ);


	/*Debug::DrawLine(startPos, startPosUp, Vector4(0, 0, 1, 1));*/

	if (!grid->FindPath(position, endPos, outPath))
	{
		return;
	}
	
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
	Vector3 up = pathNodes[0];
	up.y += 30;

	Debug::DrawLine(pathNodes[0] + pathfindingOffSet, up + pathfindingOffSet, Vector4(0, 0, 1, 1));
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


//Quaternion orientation = transform.GetLocalOrientation();

	//Quaternion rot1 = Quaternion::RotationBetweenVectors(transform.GetWorldMatrix().GetColumn(2), node - transform.GetWorldPosition());

	//Vector3 right = Vector3::Cross(node - transform.GetWorldPosition(), Vector3(0, 1, 0));
	//Vector3 up = Vector3::Cross(right, node - transform.GetWorldPosition());

	//Vector3 newUp = rot1 * Vector3(0, 1, 0);

	//Quaternion rot2 = Quaternion::RotationBetweenVectors(newUp, up);

	////transform.SetLocalOrientation(Quaternion::Slerp(orientation, rot2 * rot1, rotationSpeed));

	//Vector3 direction = node - transform.GetWorldPosition();
	//
	//float radians = atan2(-direction.y, direction.x);
	//float degrees = radians * 180.0f / 3.14159265358979323846f;

	//transform.SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), degrees));