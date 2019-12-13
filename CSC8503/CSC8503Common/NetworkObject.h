#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
namespace NCL {
	namespace CSC8503 {
		struct FullPacket : public GamePacket {
			int		objectID = -1;
			NetworkState fullState;
			int score;
			int totalScore;

			FullPacket() {
				type = Full_State;
				size = sizeof(FullPacket) - sizeof(GamePacket);
			}
		};

		struct DeltaPacket : public GamePacket {
			int		fullID		= -1;
			int		objectID	= -1;
			char	pos[3];
			char	orientation[4];

			DeltaPacket() {
				type = Delta_State;
				size = sizeof(DeltaPacket) - sizeof(GamePacket);
			}
		};

		struct ClientPacket : public GamePacket {
			int		objectID = -1;
			bool buttonstates[6];
			Quaternion	orientation;

			ClientPacket() {
				type = Received_State;
				size = sizeof(ClientPacket) - sizeof(GamePacket);
				
			}
		};

		struct CollectableCountPacket : public GamePacket {
			int count;

			CollectableCountPacket() {
				type = Collectable_Count;
				size = sizeof(CollectableCountPacket) - sizeof(GamePacket);

			}
		};

		class NetworkObject		{
		public:
			NetworkObject(GameObject& o, int id);
			virtual ~NetworkObject();

			//Called by clients
			virtual bool ReadPacket(GamePacket& p);
			//Called by servers
			virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

			void UpdateStateHistory(int minID);

			int GetID() { return networkID; };

			void increaseScore(int s) { 
				playerScore += s; 
				playerTotalScore += s;
			};

			void resetScore() { playerScore = 0; };

		protected:

			NetworkState& GetLatestNetworkState();

			bool GetNetworkState(int frameID, NetworkState& state);

			virtual bool ReadDeltaPacket(DeltaPacket &p);
			virtual bool ReadFullPacket(FullPacket &p);

			virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
			virtual bool WriteFullPacket(GamePacket**p);

			GameObject& object;

			NetworkState lastFullState;

			std::vector<NetworkState> stateHistory;

			int deltaErrors;
			int fullErrors;

			int networkID;

			int playerScore;
			int playerTotalScore;
		};
	}
}

