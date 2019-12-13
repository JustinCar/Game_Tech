#pragma once
#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"
namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c);

			void AddCollectable(GameObject* c);
			void RemoveCollectable(GameObject* c);

			Camera* GetMainCamera() const {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			void increaseScore(int s) { score += s; };
			int getScore() { return score; };

			void increasePlayerOneScore(int s) { playerOneScore += s; };
			void setPlayerOneScore(int s) { playerOneScore = s; };
			int getPlayerOneScore() { return playerOneScore; };

			void increasePlayerTwoScore(int s) { playerTwoScore += s; };
			void setPlayerTwoScore(int s) { playerTwoScore = s; };
			int getPlayerTwoScore() { return playerTwoScore; };

			void IncrementCollectableCount() { CollectableCount++; };
			void DecrementCollectableCount() { CollectableCount--; };
			void SetCollectableCount(int c) { CollectableCount = c; };
			int GetCollectableCount() { return CollectableCount; };

			void SetIsServer(bool b) { isServer = b; };
			void SetIsNetworkedGame(bool b) { isNetworkedGame = b; };

			bool GetIsServer() { return isServer; };
			bool GetIsNetworkedGame() { return isNetworkedGame; };

			void IncreasePlayerOneTotal(int i) { playerOneTotalScore += i; };
			void SetPlayerOneTotal(int i) { playerOneTotalScore = i; };
			int GetPlayerOneTotal() { return playerOneTotalScore; };

			void IncreasePlayerTwoTotal(int i) { playerTwoTotalScore += i; };
			void SetPlayerTwoTotal(int i) { playerTwoTotalScore = i; };
			int GetPlayerTwoTotal() { return playerTwoTotalScore; };
		protected:
			void UpdateTransforms();
			void UpdateQuadTree();

			std::vector<GameObject*> gameObjects;

			std::vector<Constraint*> constraints;

			int CollectableCount;

			QuadTree<GameObject*>* quadTree;

			Camera* mainCamera;

			int score;

			int playerOneScore;
			int playerTwoScore;

			int playerOneTotalScore;
			int playerTwoTotalScore;

			bool isServer;
			bool isNetworkedGame;

			bool shuffleConstraints;
			bool shuffleObjects;
		};
	}
}

