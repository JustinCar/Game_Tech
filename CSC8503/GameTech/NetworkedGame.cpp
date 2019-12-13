#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

class FullPacketReceiver : public PacketReceiver {
public:
	FullPacketReceiver(GameWorld& w, bool p, GameObject* controlled, GameObject* ghost) : world(w), isPlayerServer(p), controlledGoose(controlled), ghostGoose(ghost) {

	}

	void ReceivePacket(int type, GamePacket* payload, int source) {

		if (isPlayerServer)
			return;

		if (type == Full_State) {
			FullPacket* realPacket = (FullPacket*)payload;

			packet = realPacket->fullState;

			if (realPacket->objectID == 1000)
			{
				world.setPlayerOneScore(realPacket->score);
				world.SetPlayerOneTotal(realPacket->totalScore);
				ghostGoose->GetTransform().SetWorldPosition(packet.position);
				ghostGoose->GetTransform().SetLocalOrientation(packet.orientation);
				return;
			}
			else if (realPacket->objectID == 2000)
			{
				world.setPlayerTwoScore(realPacket->score);
				world.SetPlayerTwoTotal(realPacket->totalScore);
				controlledGoose->GetTransform().SetWorldPosition(packet.position);
				//controlledGoose->GetTransform().SetLocalOrientation(packet.orientation);
				return;
			}
			
			std::vector <GameObject*>::const_iterator first;
			std::vector <GameObject*>::const_iterator last;

			world.GetObjectIterators(first, last);

			first += realPacket->objectID;

			std::cout << "Full Packet Received..." << std::endl;

			(*first)->GetRenderObject()->SetColour(packet.colour);
			(*first)->GetTransform().SetWorldPosition(packet.position);
			(*first)->GetTransform().SetLocalOrientation(packet.orientation);
		}
	}
protected:
	GameWorld& world;
	NetworkState packet;
	bool isPlayerServer;
	GameObject* controlledGoose;
	GameObject* ghostGoose;
};

class ClientPacketReceiver : public PacketReceiver {
public:
	ClientPacketReceiver(GameWorld& w, bool p, GameObject* controlled, GameObject* ghost) : world(w), isPlayerServer(p), controlledGoose(controlled), ghostGoose(ghost) {
		speed = 500;
		jumpPower = 10000;
		swimPower = 10000;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == Received_State) {
			ClientPacket* realPacket = (ClientPacket*)payload;
			//packet = realPacket->fullState;

			GameObject* object = nullptr;

			if (realPacket->objectID == 1000 && isPlayerServer)
				object = controlledGoose;
			else if (realPacket->objectID == 1000 && !isPlayerServer)
				object = ghostGoose;

			if (realPacket->objectID == 2000 && isPlayerServer)
				object = ghostGoose;
			else if (realPacket->objectID == 2000 && !isPlayerServer)
				object = controlledGoose;

			object->GetTransform().SetLocalOrientation(realPacket->orientation);

			Vector4 z = object->GetTransform().GetWorldMatrix().GetColumn(2);

			Vector3 forward = Vector3(z.x, z.y, z.z);

			Vector4 x = object->GetTransform().GetWorldMatrix().GetColumn(0);

			Vector3 right = Vector3(x.x, x.y, x.z);


			if (realPacket->buttonstates[0] && !realPacket->buttonstates[5]) {
				object->GetPhysicsObject()->AddForce(forward * speed);
			}

			if (realPacket->buttonstates[1]) {
				object->GetPhysicsObject()->AddForce(-forward * speed);
			}

			if (realPacket->buttonstates[2]) {
				object->GetPhysicsObject()->AddForce(right * speed);
			}

			if (realPacket->buttonstates[3]) {
				object->GetPhysicsObject()->AddForce(-right * speed);
			}

			if (realPacket->buttonstates[4]) {
				object->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * jumpPower);
			}

			if (realPacket->buttonstates[0] && realPacket->buttonstates[5]) {
				object->GetPhysicsObject()->AddForce(forward * swimPower);
			}
		}
	}
protected:
	GameWorld& world;
	NetworkState packet;
	bool isPlayerServer;
	GameObject* controlledGoose;
	GameObject* ghostGoose;

	float speed;
	float jumpPower;
	float swimPower;

};

class ConnectedPacketReceiver : public PacketReceiver {
public:
	ConnectedPacketReceiver(NetworkedGame& g) : networkedGame(g) {

	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == Player_Connected) {
			ClientPacket* realPacket = (ClientPacket*)payload;

			/*if (networkedGame.GetServerPlayers().size() == 1)
				networkedGame.SpawnPlayer();*/
		}
	}
protected:
	NetworkedGame& networkedGame;
};

class CollectableCountReceiver : public PacketReceiver {
public:
	CollectableCountReceiver(GameWorld& w) : world(w) {

	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == Collectable_Count) {
			CollectableCountPacket* realPacket = (CollectableCountPacket*)payload;

			world.SetCollectableCount(realPacket->count);
		}
	}
protected:
	GameWorld& world;
};


NetworkedGame::NetworkedGame()
{
	port = NetworkBase::GetDefaultPort();
	SpawnPlayer();
}

NetworkedGame::~NetworkedGame()
{

}

void NetworkedGame::StartAsServer()
{
	ClientPacketReceiver* serverReceiver = new ClientPacketReceiver(*world, true, goose, playerTwo);
	thisServer = new GameServer(port, 2);
	thisServer->RegisterPacketHandler(Received_State, serverReceiver);
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d)
{
	thisClient = new GameClient();

	FullPacketReceiver* clientReceiver = new FullPacketReceiver(*world, isServer, goose, playerTwo);
	thisClient->RegisterPacketHandler(Full_State, &(*clientReceiver));

	CollectableCountReceiver* countReceiver = new CollectableCountReceiver(*world);
	thisClient->RegisterPacketHandler(Collectable_Count, &(*countReceiver));

	thisClient->Connect(127, 0, 0, 1, port);

	serverPlayers.insert(std::pair<int, GameObject*>(1, (GameObject*)goose));
}

void NetworkedGame::UpdateGame(float dt)
{
	
	TutorialGame::UpdateGame(dt);

	if (playerTwo && serverPlayers.size() == 1)
		serverPlayers.insert(std::pair<int, GameObject*>(2, (GameObject*)playerTwo));

	if (!isNetworkedGame)
		return;

	if (!thisServer && isServer)
		StartAsServer();

	if (!thisClient)
		StartAsClient(' ', ' ', ' ', ' ');

	if (thisServer)
		UpdateAsServer(dt);
	
	UpdateAsClient(dt);
	
}

void NetworkedGame::SpawnPlayer()
{
	float size = 1.0f;
	Player* secondPlayer = new Player(0);

	SphereVolume* volume = new SphereVolume(size);
	secondPlayer->SetBoundingVolume((CollisionVolume*)volume);

	secondPlayer->GetTransform().SetWorldScale(Vector3(size, size, size));
	secondPlayer->GetTransform().SetWorldPosition(Vector3(270, 10, 190));

	secondPlayer->setLayer(2);
	secondPlayer->setLayerMask(0);

	playerTwo = secondPlayer;

	serverPlayers.insert(std::pair<int, GameObject*>(2, (GameObject*)secondPlayer));
}

void NetworkedGame::StartLevel()
{

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{

}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b)
{

}

void NetworkedGame::UpdateAsServer(float dt)
{
	thisServer->UpdateServer();
	BroadcastSnapshot(false);

	CollectableCountPacket* packet = new CollectableCountPacket();
	packet->count = world->GetCollectableCount();

	thisServer->SendPacketToPeer(*packet, 2);
	delete packet;

}

void NetworkedGame::UpdateAsClient(float dt) {
	thisClient->UpdateClient();

	if (isServer)
		return;

	ClientPacket newPacket;

	for (int i = 0; i < 6; i++) 
	{
		newPacket.buttonstates[i] = false;
	}

	newPacket.orientation = goose->GetTransform().GetLocalOrientation();

	if (isServer)
	{
		newPacket.objectID = 1000;

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
			newPacket.buttonstates[0] = true;
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
			newPacket.buttonstates[1] = true;
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
			newPacket.buttonstates[2] = true;

		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
			newPacket.buttonstates[3] = true;
		}
	}
	else
	{
		newPacket.objectID = 2000;

		bool* buttonStates = goose->getButtonStates();

		if (buttonStates[0]) {
			newPacket.buttonstates[0] = true;
		}

		if (buttonStates[1]) {
			newPacket.buttonstates[1] = true;
		}

		if (buttonStates[2]) {
			newPacket.buttonstates[2] = true;

		}

		if (buttonStates[3]) {
			newPacket.buttonstates[3] = true;
		}

		if (buttonStates[4]) {

			newPacket.buttonstates[4] = true;
		}

		if (buttonStates[5]) {
			newPacket.buttonstates[5] = true;
		}
	}
	
	thisClient -> SendPacket(newPacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;
	
	world -> GetObjectIterators(first, last);
	
	for (int j = 0; j < serverPlayers.size(); j++) 
	{
		
		for (auto i = first; i != last; ++i) {
			NetworkObject * o = (*i) -> GetNetworkObject();
			if (!o) {
				continue;
			}
			int playerState = 0; // You ’ll need to do this bit !
			GamePacket * newPacket = nullptr;
			if (o -> WritePacket(&newPacket, deltaFrame, playerState)) {
				//thisServer -> SendGlobalPacket(*newPacket); // change ...

				thisServer->SendPacketToPeer(*newPacket, j + 1);
				delete newPacket;
			}
		}
	}
}
