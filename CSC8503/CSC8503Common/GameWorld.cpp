#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "../../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld()	{
	mainCamera = new Camera();

	quadTree = nullptr;

	score = 0;

	playerOneScore = 0;
	playerTwoScore = 0;

	playerOneTotalScore = 0;
	playerTwoTotalScore = 0;

	isServer = false;
	isNetworkedGame = false;

	shuffleConstraints	= false;
	shuffleObjects		= false;
}

GameWorld::~GameWorld()	{
}

void GameWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
	score = 0;
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();
	score = 0;
	playerOneScore = 0;
	playerTwoScore = 0;
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
}

void GameWorld::RemoveGameObject(GameObject* o) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
}

void GameWorld::GetObjectIterators(
	GameObjectIterator& first,
	GameObjectIterator& last) const {

	first	= gameObjects.begin();
	last	= gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
	}
}

void GameWorld::UpdateWorld(float dt) {
	UpdateTransforms();

	for (auto& i : gameObjects) {
		
		if (isNetworkedGame)
		{
			if (isServer && i->GetNetworkObject()->GetID() == 2000)
				continue;

			if (!isServer && i->GetNetworkObject()->GetID() == 1000)
				continue;

			i->Update(dt);
		} else
			i->Update(dt);
	}

	if (shuffleObjects) {
		std::random_shuffle(gameObjects.begin(), gameObjects.end());
	}

	if (shuffleConstraints) {
		std::random_shuffle(constraints.begin(), constraints.end());
	}
}

void GameWorld::UpdateTransforms() {
	for (auto& i : gameObjects) {
		i->GetTransform().UpdateMatrices();
	}
}

void GameWorld::UpdateQuadTree() {
	delete quadTree;

	//quadTree = new QuadTree<GameObject*>(Vector2(512, 512), 6);

	//for (auto& i : gameObjects) {
	//	quadTree->Insert(i);
	//}
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject) const {
	//The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;

	for (auto& i : gameObjects) {
		if (!i->GetBoundingVolume()) { //objects might not be collideable etc...
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
				
			if (!closestObject) {	
				closestCollision		= collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	if (collision.node) {
		closestCollision		= collision;
		closestCollision.node	= collision.node;
		return true;
	}
	return false;
}


/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin();
	last	= constraints.end();
}