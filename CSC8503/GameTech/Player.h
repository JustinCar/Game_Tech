#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../../Common/Window.h"
#include "../../Common/Camera.h"
#include <cmath>
#define PI 3.14159265
namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject
		{



		public:
			Player();
			~Player();
			void UpdatePlayer(float dt);

			void setCamera(Camera* c) { mainCamera = c; };

			float yaw;
			
		protected:
			void UpdateKeys(float dt);
			void UpdateCamera(float dt);
			float speed;
			Camera* mainCamera;
			Vector3 camOffset;
			float jumpPower;
		};
	}
}
