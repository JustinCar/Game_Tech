#include "Collectable.h"

using namespace NCL;
using namespace CSC8503;

Collectable::Collectable(int p, Vector3 pos) : points(p), originalPosition(pos)
{
	layer = 4;
	layerMask = 4;
}

Collectable::~Collectable()
{

}

void Collectable::Update(float dt)
{

}

void Collectable::Trigger(GameObject& obj)
{
	layerMask = 6; // Prevent further collisions with player

	Player* p = static_cast<Player*>(&obj);

	p->getCollectables().push(this);
}