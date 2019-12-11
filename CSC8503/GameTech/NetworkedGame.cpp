#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

class FullPacketReceiver : public PacketReceiver {
public:
	FullPacketReceiver(GameWorld& w, bool p) : world(w), isPlayerServer(p) {

	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == Full_State) {
			FullPacket* realPacket = (FullPacket*)payload;

			packet = realPacket->fullState;
			
			std::vector <GameObject*>::const_iterator first;
			std::vector <GameObject*>::const_iterator last;

			world.GetObjectIterators(first, last);

			first += realPacket->objectID;

			std::cout << "Full Packet Received..." << std::endl;

			(*first)->GetTransform().SetWorldPosition(packet.position);
			(*first)->GetTransform().SetWorldOrientation(packet.orientation);
		}
	}
protected:
	GameWorld& world;
	NetworkState packet;
	bool isPlayerServer;
};

class ClientPacketReceiver : public PacketReceiver {
public:
	ClientPacketReceiver(GameWorld& w) : world(w) {

	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == Received_State) {
			ClientPacket* realPacket = (ClientPacket*)payload;
			//packet = realPacket->fullState;

			std::vector <GameObject*>::const_iterator first;
			std::vector <GameObject*>::const_iterator last;

			world.GetObjectIterators(first, last);

			first += packet.stateID;

			(*first)->GetTransform().SetWorldPosition(packet.position);
			(*first)->GetTransform().SetWorldOrientation(packet.orientation);
		}
	}
protected:
	GameWorld& world;
	NetworkState packet;
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
	ClientPacketReceiver* serverReceiver = new ClientPacketReceiver(*world);
	thisServer = new GameServer(port, 2);
	thisServer->RegisterPacketHandler(Received_State, &(*serverReceiver));

}

void NetworkedGame::StartAsClient(char a, char b, char c, char d)
{
	FullPacketReceiver* clientReceiver;

	thisClient = new GameClient();

	if (!isServer)
	{
		clientReceiver = new FullPacketReceiver(*world, true);
		thisClient->RegisterPacketHandler(Full_State, &(*clientReceiver));
	}
	
	thisClient->Connect(127, 0, 0, 1, port);

	serverPlayers.insert(std::pair<int, GameObject*>(1, (GameObject*)goose));
}

void NetworkedGame::UpdateGame(float dt)
{
	
	TutorialGame::UpdateGame(dt);

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
	float inverseMass = 0.1f;

	GameObject* secondPlayer = new GameObject();

	SphereVolume* volume = new SphereVolume(size);
	secondPlayer->SetBoundingVolume((CollisionVolume*)volume);

	secondPlayer->GetTransform().SetWorldScale(Vector3(size, size, size));
	secondPlayer->GetTransform().SetWorldPosition(Vector3(270, 10, 190));

	secondPlayer->SetRenderObject(new RenderObject(&secondPlayer->GetTransform(), gooseMesh, nullptr, basicShader));
	secondPlayer->SetPhysicsObject(new PhysicsObject(&secondPlayer->GetTransform(), secondPlayer->GetBoundingVolume()));

	secondPlayer->GetPhysicsObject()->SetInverseMass(inverseMass);
	secondPlayer->GetPhysicsObject()->InitSphereInertia();

	int id;
	if (playerID == 1000)
		id = 2000;
	else
		id = 1000;

	NetworkObject* o = new NetworkObject(*secondPlayer, id);

	secondPlayer->SetNetworkObject(o);

	world->AddGameObject(secondPlayer);

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
}

void NetworkedGame::UpdateAsClient(float dt) {
	thisClient->UpdateClient();

	ClientPacket newPacket;
	
	if (Window::GetKeyboard() -> KeyPressed(KeyboardKeys::SPACE)) {
		// fire button pressed !
		newPacket.buttonstates[0] = 1;
		newPacket.lastID = 0; // You ’ll need to work this out somehow ...
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
