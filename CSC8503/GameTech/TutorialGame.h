#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "Player.h"
#include "Enemy.h"
#include <stdlib.h>
#include "Collectable.h"
#include <string> 
#include <iostream>
#include <fstream>

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void ResetCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void SimpleGJKTest();

			bool SelectObject();
			void SeenObjects();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void LockedCameraMovement();

			GameObject* AddFloorToWorld(const Vector3& position);

			void AddObstacles();

			GameObject* AddTerrainToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);
			GameObject* AddLakeToWorld(const Vector3& position, const Vector3& size, const Vector4& colour);

			void AddBridgeToWorld(Vector3 startPos, int num);

			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			//IT'S HAPPENING
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddPlayerTwoToWorld(const Vector3& position);
			Enemy*		AddParkKeeperToWorld(const Vector3& position);
			GameObject* AddCharacterToWorld(const Vector3& position);
			GameObject* AddAppleToWorld(const Vector3& position);
			GameObject* AddBonusItemToWorld(const Vector3& position);

			Player* goose;
			GameObject* playerTwo;
			vector<Enemy*> enemies;

			void StoreHighScore();
			std::string fileName;

			int secondPlayerScore;

			void RenderMenu();
			void RenderScoreBoard();
			int buttonSelected;
			bool playing;
			void StartGame();

			bool isNetworkedGame;
			bool isServer;
			int playerID;

			void RestartNetworkedGame();

			bool newSession;

			float matchTimer;
			float gameOverScreenCoolDown;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float	forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			NavigationGrid grid;

			//Coursework Meshes
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLMesh*	charA		= nullptr;
			OGLMesh*	charB		= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
		};
	}
}

