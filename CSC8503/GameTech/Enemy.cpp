#include "Enemy.h"

using namespace NCL;
using namespace CSC8503;

Enemy::Enemy(Vector3 position, GameWorld* world, bool& isServer) : gameWorld(world), isServerEnemy(isServer), GameObject("ENEMY")
{
	speed = 80;
	chaseSpeed = 200;
	rotationSpeed = 5;
	player = nullptr;
	playerTwo = nullptr;
	closestPlayer = player;
	grid = new NavigationGrid("TestGrid3.txt");

	attackTimer = 0;
	attackCooldown = 10;

	position.x = roundToNearestTen(position.x);
	position.z = roundToNearestTen(position.z);

	layer = 3;
	layerMask = 4;

	while (!grid->ValidStartingPosition(position))
	{
		int xPos = rand() % 480;
		int zPos = rand() % 420;
		position = Vector3(xPos, 12, zPos);
	}

	transform.SetWorldPosition(position);

	timeToSpendIdle = rand() % 10 + 2;
	timeToSpendPatrolling = rand() % 20 + 5;

	timeSpentIdle = 0.0f;
	timeSpentPatrolling = 0.0f;

	distanceFromPlayer = 0.0f;
	chaseRadius = 30.0f;
	attackRadius = 10.0f;

	pathfindingOffSet = Vector3(0, 10, 0);
	index = 0;
	currentDirection = Vector3(0, 0, 0);

	InitStateMachine();
}

Enemy::~Enemy()
{

}

void Enemy::resetPosition()
{
	
	int xPos = rand() % 480;
	int zPos = rand() % 420;
	Vector3 position(xPos, 0, zPos);

	while (!grid->ValidStartingPosition(position))
	{
		xPos = rand() % 480;
		zPos = rand() % 420;
		position = Vector3(xPos, 12, zPos);
	}

	transform.SetWorldPosition(position);
}

void Enemy::Update(float dt)
{
	if (!isServerEnemy)
		return;

	attackTimer -= dt;

	if (player && playerTwo)
	{
		if (Vector3::Distance(transform.GetWorldPosition(), player->GetTransform().GetWorldPosition()) >
			Vector3::Distance(transform.GetWorldPosition(), playerTwo->GetTransform().GetWorldPosition()))
		{
			closestPlayer = playerTwo;
			distanceFromPlayer = Vector3::Distance(transform.GetWorldPosition(), playerTwo->GetTransform().GetWorldPosition());
		}
		else 
		{
			closestPlayer = player;
			distanceFromPlayer = Vector3::Distance(transform.GetWorldPosition(), player->GetTransform().GetWorldPosition());
		}
	}

	// Prevent enemy from continuing to chase after attacking
	if (attackTimer > 0 || !grid->ValidStartingPosition(player->GetTransform().GetWorldPosition()))
		distanceFromPlayer = 1000;
	
		

	stateMachine->Update(dt);
}

void Enemy::Trigger(GameObject& obj)
{

}

void Enemy::InitStateMachine()
{
	stateMachine = new StateMachine();

	// Hierarchical State Machine
	StateFunc patrol = [](void* data, float dt) {
		EnemyPatrolSuperState* realData = (EnemyPatrolSuperState*)data;
		realData->Update(dt);
	};

	StateFunc chase = [](void* data, float dt) {
		EnemyChaseSuperState* realData = (EnemyChaseSuperState*)data;
		realData->Update(dt);
	};

	EnemyPatrolSuperState* patrolSuperState =  new EnemyPatrolSuperState(*this);
	EnemyChaseSuperState* chaseSuperState = new EnemyChaseSuperState(*this);

	GenericState* stateA = new GenericState(patrol, (void*) patrolSuperState);
	GenericState* stateB = new GenericState(chase, (void*) chaseSuperState);
	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	GenericTransition <float&, float>* transitionA =
		new GenericTransition <float&, float >(
			GenericTransition <float&, float >::LessThanTransition,
			distanceFromPlayer, chaseRadius, stateA, stateB);

	GenericTransition <float&, float>* transitionB =
		new GenericTransition <float&, float >(
			GenericTransition <float&, float >::GreaterThanTransition,
			distanceFromPlayer, chaseRadius, stateB, stateA);

	stateMachine->AddTransition(transitionA);
	stateMachine->AddTransition(transitionB);
}

void Enemy::Chase(float dt)
{

	Player* p = static_cast<Player*>(&(*closestPlayer));

	if (p->IsSwimming())
		return;

	Vector3 playerPos = closestPlayer->GetTransform().GetWorldPosition();

	RotateTowards(playerPos);

	Vector3 pos = transform.GetWorldPosition();
	playerPos.y = pos.y;
	Vector3 dir = playerPos - pos;
	dir.Normalise();

	physicsObject->AddForce(dir * speed);
}

void Enemy::Attack(float dt)
{
	attackTimer = attackCooldown;

	Player* p = static_cast<Player*>(&(*closestPlayer));

	for (int i = 0; i < p->getCollectables().size(); i++)
	{
		Collectable* c = static_cast<Collectable*>(&(*p->getCollectables().front()));

		gameWorld->RemoveConstraint(c->GetConstraint());
		c->GetTransform().SetWorldPosition(c->GetOriginalPosition());
		c->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
		c->setLayerMask(4);

		p->getCollectables().pop();
	}

	Vector3 dir = p->GetTransform().GetWorldPosition() - transform.GetWorldPosition();
	dir.Normalise();

	p->GetPhysicsObject()->AddForce(dir * 10000);

	pathNodes.clear();
	index = 0;
	GeneratePath();
}

void Enemy::Idle(float dt)
{
	if (pathNodes.size() > 0)
	{
		pathNodes.clear();
		timeSpentPatrolling = 0;
		index = 0;
	}

	timeSpentIdle += dt;
}

void Enemy::Patrol(float dt)
{
	if (pathNodes.size() == 0)
	{
		GeneratePath();
		if (pathNodes.size() == 0)
			return;
		timeSpentIdle = 0;
	}

	timeSpentPatrolling += dt;

	//DisplayPathfinding();

	Vector3 node = pathNodes[index];

	node += pathfindingOffSet;

	Vector3 pos = transform.GetWorldPosition();

	node.y = pos.y;

	Vector3 dir = node - pos;
	dir.Normalise();

	physicsObject->AddForce(dir * speed);

	RotateTowards(node);

	if (Vector3::Distance(pos, node) <= 1)
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

void Enemy::RotateTowards(Vector3 v)
{

	Vector3 direction = v - transform.GetWorldPosition();
	
	float radians = atan2(direction.x, direction.z);
	float degrees = radians * 180.0f / 3.14159265358979323846f;
	
	transform.SetLocalOrientation(Quaternion::Slerp(transform.GetLocalOrientation(), Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), degrees), rotationSpeed));
}
