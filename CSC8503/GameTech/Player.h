#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../../Common/Window.h"
#include "../../Common/Camera.h"
#include <cmath>
#include <list> 
#include <queue>
#include "Collectable.h"
#define PI 3.14159265
namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject
		{
		public:
			Player();
			~Player();
			//void UpdatePlayer(float dt);
			virtual void Update(float dt);
			virtual void Trigger(GameObject& obj);

			void setCamera(Camera* c) { mainCamera = c; };

			float yaw;

			std::queue<GameObject*>& getCollectables() { return collectables; };

			virtual void OnCollisionBegin(GameObject* otherObject);

			virtual void OnCollisionEnd(GameObject* otherObject);

			bool IsSwimming() { return isSwimming; };
			
		protected:
			void UpdateKeys(float dt);
			void UpdateCamera(float dt);
			float speed;

			//vector<GameObject> collectables;
			float swimPower;
			float swimTimer;
			float swimCoolDown;
			bool isSwimming;
			Camera* mainCamera;
			Vector3 camOffset;
			float jumpPower;
			float jumpTimer;
			float jumpCoolDown;

			std::queue<GameObject*> collectables;
		};
	}
}
