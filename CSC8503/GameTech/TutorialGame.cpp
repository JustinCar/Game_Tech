#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/PositionConstraint.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	newSession = true;

	fileName = "highscores";

	goose = nullptr;

	matchTimer = -1;
	gameOverScreenCoolDown = 20.0f;

	buttonSelected = 1;
	playing = false;

	playerID = 0;

	isNetworkedGame = false;
	isServer = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("goose.msh"	 , &gooseMesh);
	loadFunc("CharacterA.msh", &keeperMesh);
	loadFunc("CharacterM.msh", &charA);
	loadFunc("CharacterF.msh", &charB);
	loadFunc("Apple.msh"	 , &appleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete basicTex;
	delete basicShader;

	delete goose;
	for (int i = 0; i < enemies.size(); i++) 
	{
		delete enemies[i];
	}

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::StartGame()
{
	matchTimer = 180.0f;
	InitCamera();
	InitWorld();

	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;

	world->GetObjectIterators(first, last);

	int counter = 0;

	for (auto i = first; i != last; ++i) 
	{
		/*if ((*i)->getLayer() == 2)
		{
			(*i)->SetNetworkObject(new NetworkObject(*(*i), playerID));
			continue;
		}*/
		
		if (!(*i)->GetNetworkObject())
			(*i)->SetNetworkObject(new NetworkObject(*(*i), counter));
		counter++;
	}
}

void TutorialGame::RestartNetworkedGame()
{
	//StoreHighScore();
	newSession = false;
	playing = true;
	world->setPlayerOneScore(0);
	world->setPlayerTwoScore(0);
	Vector3 offSet(275, 10, 195);
	matchTimer = 100;
	goose->GetNetworkObject()->resetScore();
	playerTwo->GetNetworkObject()->resetScore();
	physics->Clear();

	if (isServer)
	{
		goose->GetTransform().SetWorldPosition(offSet + Vector3(5, 0, 5));
		playerTwo->GetTransform().SetWorldPosition(offSet - Vector3(5, 0, 5));
	}

	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;

	world->GetObjectIterators(first, last);

	int collectableCounter = 0;

	for (auto i = first; i != last; ++i)
	{
		// Reset Collectables
		if ((*i)->getLayer() == 4)
		{
			(*i)->setLayerMask(4);
			int xPos = rand() % 480;
			int zPos = rand() % 420;
			(*i)->GetTransform().SetWorldPosition(Vector3(xPos, 10, zPos));
			(*i)->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
			collectableCounter++;
		}

		// Reset Enemies
		if ((*i)->getLayer() == 3)
		{
			Enemy* e = (Enemy*)(*i);
			e->resetPosition();
		}
	}

	world->SetCollectableCount(collectableCounter);
}

void TutorialGame::RenderScoreBoard()
{
	if (isServer)
	{
		renderer->DrawString("YOUR TOTAL SCORE: " + std::to_string(world->GetPlayerOneTotal()),
			Vector2(400, 400), Vector4(0, 0, 1, 1));
		renderer->DrawString("THEIR TOTAL SCORE: " + std::to_string(world->GetPlayerTwoTotal()),
			Vector2(400, 350), Vector4(1, 0, 0, 1));
	}
	else
	{
		renderer->DrawString("YOUR TOTAL SCORE: " + std::to_string(world->GetPlayerTwoTotal()),
			Vector2(400, 400), Vector4(1, 0, 0, 1));
		renderer->DrawString("THEIR TOTAL SCORE: " + std::to_string(world->GetPlayerOneTotal()),
			Vector2(400, 350), Vector4(0, 0, 1, 1));
	}
}

void TutorialGame::StoreHighScore()
{
	std::ofstream myfile;
	myfile.open(fileName, std::ifstream::out, std::ifstream::trunc);

	myfile << "HIGH SCORES FROM LAST MATCH" << "\n";

	myfile << "Player One (Blue): " << std::to_string(world->GetPlayerOneTotal()) << "\n";

	myfile << "Player Two (Red): " << std::to_string(world->GetPlayerTwoTotal()) << "\n";

	myfile.close();
}

void TutorialGame::RenderMenu()
{
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN) && buttonSelected != 4) {

		buttonSelected++;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP) && buttonSelected != 1) {

		buttonSelected--;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RETURN)) {

		switch (buttonSelected)
		{
		case 1: playing = true;
				playerID = 1000;
				isServer = true;
				StartGame();
				return;
		case 2: playerID = 2000;
				playing = true;
				isNetworkedGame = true;
				world->SetIsNetworkedGame(true);
				StartGame();
				return;
		case 3: playerID = 1000;
				playing = true;
				isNetworkedGame = true;
				world->SetIsServer(true);
				world->SetIsNetworkedGame(true);
				isServer = true;
				StartGame();
				return;
		case 4: exit(0);
		}
	}

	switch (buttonSelected) 
	{
	case 1 : renderer->DrawString("Play Singleplayer",
					Vector2(40, 600), Vector4(0, 0, 1, 1));
			renderer->DrawString("Play Multiplayer",
						Vector2(40, 500));
			renderer->DrawString("Play Multiplayer as Server",
				Vector2(40, 400));
			renderer->DrawString("Exit",
				Vector2(40, 300));
			
	case 2 : renderer->DrawString("Play Singleplayer",
				Vector2(40, 600));
			renderer->DrawString("Play Multiplayer",
				Vector2(40, 500), Vector4(0, 0, 1, 1));
			renderer->DrawString("Play Multiplayer as Server",
				Vector2(40, 400));
			renderer->DrawString("Exit",
				Vector2(40, 300));

	case 3 : renderer->DrawString("Play Singleplayer",
				Vector2(40, 600));
			renderer->DrawString("Play Multiplayer",
				Vector2(40, 500));
			renderer->DrawString("Play Multiplayer as Server",
				Vector2(40, 400), Vector4(0, 0, 1, 1));
			renderer->DrawString("Exit",
				Vector2(40, 300));
	case 4: renderer->DrawString("Play Singleplayer",
			Vector2(40, 600));
			renderer->DrawString("Play Multiplayer",
				Vector2(40, 500));
			renderer->DrawString("Play Multiplayer as Server",
				Vector2(40, 400));
			renderer->DrawString("Exit",
				Vector2(40, 300), Vector4(0, 0, 1, 1));
	
	}
}

void TutorialGame::UpdateGame(float dt) {

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	if (!playing)
	{
		if (matchTimer > 0)
		{
			renderer->DrawString("!!GAMEOVER!!",
						Vector2(450, 600), Vector4(0, 0, 1, 1));

			if (isNetworkedGame)
			{
				if (isServer)
				{
					renderer->DrawString("YOUR FINAL SCORE: " + std::to_string(world->getPlayerOneScore()),
						Vector2(425, 400), Vector4(0, 0, 1, 1));
					renderer->DrawString("THEIR FINAL SCORE: " + std::to_string(world->getPlayerTwoScore()),
						Vector2(425, 350), Vector4(1, 0, 0, 1));
				}
				else
				{
					renderer->DrawString("YOUR FINAL SCORE: " + std::to_string(world->getPlayerTwoScore()),
						Vector2(425, 400), Vector4(1, 0, 0, 1));
					renderer->DrawString("THEIR FINAL SCORE: " + std::to_string(world->getPlayerOneScore()),
						Vector2(425, 350), Vector4(0, 0, 1, 1));
				}

				if (world->getPlayerOneScore() > world->getPlayerTwoScore())
				{
					renderer->DrawString("!!BLUE WINS!!",
						Vector2(425, 300), Vector4(0, 0, 1, 1));
				}
				else if (world->getPlayerOneScore() < world->getPlayerTwoScore())
				{
					renderer->DrawString("!!RED WINS!!",
						Vector2(425, 300), Vector4(1, 0, 0, 1));
				}
				else
				{
					renderer->DrawString("!!DRAW!!",
						Vector2(425, 300), Vector4(1, 0, 1, 1));
				}

				renderer->DrawString("TIME TILL NEXT MATCH: " + std::to_string(matchTimer),
					Vector2(325, 200), Vector4(1, 0, 1, 1));
			} else 
			{
				renderer->DrawString("FINAL SCORE: " + std::to_string(world->getScore()),
					Vector2(425, 400), Vector4(0, 0, 1, 1));
			}
			
			
			matchTimer -= dt;
		}
		else if (!isNetworkedGame && matchTimer <= 0 && world->getScore() > 0)
		{
			physics->Clear();
			world->ClearAndErase();
		} 
		else if (isNetworkedGame && matchTimer <= 0)
		{
			if (isServer)
				RestartNetworkedGame();
			else
			{
				newSession = false;
				playing = true;
				matchTimer = 100;
				world->SetCollectableCount(1);
			}
		}
		else if (!isNetworkedGame)
		{
			RenderMenu();
		}
	}
	else
	{

		if (!isNetworkedGame)
		{
			matchTimer -= dt;
			int seconds = matchTimer;
			renderer->DrawString(std::to_string(seconds / 60) + "." + std::to_string(seconds % 60),
				Vector2(640, 600), Vector4(0, 0, 1, 1));

			renderer->DrawString("SCORE: " + std::to_string(world->getScore()),
						Vector2(50, 600), Vector4(0, 0, 1, 1));
		}
		else 
		{
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::TAB))
				RenderScoreBoard();

			if (isServer) 
			{
				renderer->DrawString("YOUR SCORE: " + std::to_string(world->getPlayerOneScore()),
					Vector2(50, 600), Vector4(0, 0, 1, 1));
				renderer->DrawString("THEIR SCORE: " + std::to_string(world->getPlayerTwoScore()),
					Vector2(50, 550), Vector4(1, 0, 0, 1));
			}
			else 
			{
				renderer->DrawString("YOUR SCORE: " + std::to_string(world->getPlayerTwoScore()),
					Vector2(50, 600), Vector4(1, 0, 0, 1));
				renderer->DrawString("THEIR SCORE: " + std::to_string(world->getPlayerOneScore()),
					Vector2(50, 550), Vector4(0, 0, 1, 1));
			}
		}

		// Gameover
		if ((matchTimer <= 0) || (world->GetCollectableCount() == 0))
		{
			playing = false;
			matchTimer = gameOverScreenCoolDown;
		}

	}
	
	if (lockedObject != nullptr) {
		LockedCameraMovement();
	}

	UpdateKeys();

	SelectObject();
	MoveSelectedObject();
	//SeenObjects();

	Debug::DrawLine(Vector3(0, 0, 0), Vector3(0, 50, 0), Vector4(1, 0, 0, 1));
	Debug::DrawLine(Vector3(480, 0, 0), Vector3(480, 50, 0), Vector4(1, 0, 0, 1));
	Debug::DrawLine(Vector3(480, 0, 420), Vector3(480, 50, 420), Vector4(1, 0, 0, 1));
	Debug::DrawLine(Vector3(0, 0, 420), Vector3(0, 50, 420), Vector4(1, 0, 0, 1));

	world->UpdateWorld(dt);
	renderer->Update(dt);

	if (!isNetworkedGame)
		physics->Update(dt);
	else if (isServer && isNetworkedGame)
		physics->Update(dt);

	Debug::FlushRenderables();
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}
}

void  TutorialGame::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				//selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));

				renderer->DrawString("DEBUG INFO: ",
					Vector2(10, 300), Vector4(0, 0, 1, 1));
				renderer->DrawString("POSITION: " + selectionObject->GetTransform().GetWorldPosition().ToString(),
					Vector2(10, 200), Vector4(0, 0, 1, 1));
				renderer->DrawString("SIZE: " + selectionObject->GetTransform().GetLocalScale().ToString(),
					Vector2(10, 100), Vector4(0, 0, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
}

void TutorialGame::SeenObjects() {
	

	GameObjectIterator first;
	GameObjectIterator last;
	world->GetObjectIterators(first, last);

	for (vector<GameObject*>::const_iterator i = first;
			i != last; i++) {
	
		Ray ray = CollisionDetection::BuildRayToCamera(*world->GetMainCamera(), **i);

		GameObject* obj = *i;

		RayCollision closestCollision;
		if (!world->Raycast(ray, closestCollision, true)) {

			obj->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
		}
		else 
		{
			obj->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
		}
	}
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	renderer -> DrawString(" Click Force :" + std::to_string(forceMagnitude),
		Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse() -> GetWheelMovement() * 100.0f;
	
	if (!selectionObject) {
		return;// we haven �t selected anything !

	}
	// Push the selected object !
	if (Window::GetMouse() -> ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(
			* world -> GetMainCamera());
		
		RayCollision closestCollision;
		if (world -> Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject -> GetPhysicsObject() -> AddForceAtPosition(
					ray.GetDirection() * forceMagnitude,
					closestCollision.collidedAt);
				
			}

		}

	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::ResetCamera() {
	world->GetMainCamera()->SetNearPlane(0.0f);
	world->GetMainCamera()->SetFarPlane(0.0f);
	world->GetMainCamera()->SetPitch(0.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(0, 0, 0));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	grid = NavigationGrid("TestGrid3.txt");

	Vector3 offSet(220, 0, 195);

	AddGooseToWorld(offSet + Vector3(50, 10, 0));

	if (isNetworkedGame)
		AddPlayerTwoToWorld(offSet + Vector3(50, 10, 0));

	for (int i = 0; i < 10; i++)
	{
		int xPos = rand() % 480;
		int zPos = rand() % 420;

		AddAppleToWorld(Vector3(xPos, 40, zPos));
		world->IncrementCollectableCount();
	}

	for (int i = 0; i < 5; i++)
	{
		int xPos = rand() % 480;
		int zPos = rand() % 420;

		AddBonusItemToWorld(Vector3(xPos, 40, zPos));
		world->IncrementCollectableCount();
	}

	for (int i = 0; i < 8; i++) 
	{
		int xPos = rand() % 480;
		int zPos = rand() % 420;
		enemies.push_back(AddParkKeeperToWorld(Vector3(xPos, 12, zPos)));
	}

	Vector3 westBridgeStartPos = Vector3(42, 8, 15);
	Vector3 eastBridgeStartPos = Vector3(-100, 8, 15);
	Vector3 bigBridgeStartPos = Vector3(-50, 8, -35);

	AddBridgeToWorld(offSet + westBridgeStartPos, 1);
	AddBridgeToWorld(offSet + eastBridgeStartPos, 2);
	AddBridgeToWorld(offSet + bigBridgeStartPos, 3);

	Vector4 green = Vector4(0, 0.6, 0, 1);
	Vector4 blue = Vector4(0, 0, 1, 1);
	Vector4 grey = Vector4(0.41, 0.41, 0.41, 1);
	Vector4 brown = Vector4(0.58, 0.29, 0, 1);

	AddObstacles();

	AddTerrainToWorld(offSet + Vector3(180, -12, 15), Vector3(80, 20, 50), green); // West floor
	AddTerrainToWorld(offSet + Vector3(-140, -12, 15), Vector3(80, 20, 50), green); // East floor
	AddTerrainToWorld(offSet + Vector3(20, -12, -115), Vector3(240, 20, 80), green); // South floor
	AddTerrainToWorld(offSet + Vector3(20, -12, 145), Vector3(240, 20, 80), green); // North floor

	AddTerrainToWorld(offSet + Vector3(260, 98, 15), Vector3(2, 100, 240), brown); // West wall
	AddTerrainToWorld(offSet + Vector3(-220, 98, 15), Vector3(2, 100, 240), brown); // East wall
	AddTerrainToWorld(offSet + Vector3(20, 98, 225), Vector3(240, 100, 2), brown); // South wall
	AddTerrainToWorld(offSet + Vector3(20, 98, -195), Vector3(240, 100, 2), brown); // North wall

	GameObject* island = AddTerrainToWorld(offSet + Vector3(20, -11, 15), Vector3(20, 20, 20), green); // Island
	island->setLayer(5);
	island->setLayerMask(49);
	
	AddLakeToWorld(offSet + Vector3(20, -12, 15), Vector3(80, 20, 50), Vector4(0, 0.41, 0.58, 1)); // Lake
}

//From here on it's functions to add in objects to the world!
void TutorialGame::AddObstacles()
{
	Vector4 brown = Vector4(0.58, 0.29, 0, 1);

	AddTerrainToWorld(Vector3(430, 18, 150), Vector3(55, 10, 5), brown);
	AddTerrainToWorld(Vector3(380, 18, 165), Vector3(5, 10, 10), brown);
	AddTerrainToWorld(Vector3(360, 18, 170), Vector3(15, 10, 5), brown);
	AddTerrainToWorld(Vector3(430, 18, 180), Vector3(5, 10, 15), brown);
	AddTerrainToWorld(Vector3(405, 18, 190), Vector3(30, 10, 5), brown);
	AddTerrainToWorld(Vector3(380, 18, 210), Vector3(5, 10, 15), brown);
	AddTerrainToWorld(Vector3(365, 18, 230), Vector3(45, 10, 5), brown);
	AddTerrainToWorld(Vector3(455, 18, 230), Vector3(20, 10, 5), brown);

	Vector3 offset(-310, 0, 10);

	AddTerrainToWorld(Vector3(20, 18, 140), Vector3(30, 10, 5), brown);
	AddTerrainToWorld(offset + Vector3(430, 18, 150), Vector3(55, 10, 5), brown);
	AddTerrainToWorld(offset + Vector3(380, 18, 165), Vector3(5, 10, 10), brown);
	AddTerrainToWorld(offset + Vector3(360, 18, 170), Vector3(15, 10, 5), brown);
	AddTerrainToWorld(offset + Vector3(430, 18, 180), Vector3(5, 10, 15), brown);
	AddTerrainToWorld(offset + Vector3(405, 18, 190), Vector3(30, 10, 5), brown);
	AddTerrainToWorld(offset + Vector3(380, 18, 210), Vector3(5, 10, 15), brown);
	AddTerrainToWorld(offset + Vector3(365, 18, 230), Vector3(45, 10, 5), brown);
	AddTerrainToWorld(offset + Vector3(455, 18, 230), Vector3(20, 10, 5), brown);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("FLOOR");

	Vector3 floorSize = Vector3(300, 2, 300);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, nullptr, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetRenderObject()->SetColour(Vector4(0, 0.6, 0, 1));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddTerrainToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* floor = new GameObject("FLOOR");

	floor->setLayer(1);
	floor->setLayerMask(49);

	AABBVolume* volume = new AABBVolume(size);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(size);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetRenderObject()->SetColour(colour);

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddLakeToWorld(const Vector3& position, const Vector3& size, const Vector4& colour) {
	GameObject* floor = new GameObject("LAKE");

	floor->setLayer(6);
	floor->setLayerMask(49);

	Vector3 colliderSize = size;
	colliderSize.y -= 1.2;
	AABBVolume* volume = new AABBVolume(colliderSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(size);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetRenderObject()->SetColour(colour);

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

void TutorialGame::AddBridgeToWorld(Vector3 startPos, int num) {
	Vector3 cubeSize = Vector3(1, 1, 10);

	float	invCubeMass = 0.01;
	int		numLinks = 55;
	float	maxDistance = 1.2;
	float	cubeDistance = 2;

	int layerMask = 53;

	Vector4 brown = Vector4(0.58, 0.29, 0, 1);

	//Vector3 startPos = Vector3(36, 7, 15);

	Vector3 endPos = startPos;

	if (num == 1)
	{
		endPos.y += 19;
	}
	else if (num == 2)
	{
		startPos.y += 19;
		numLinks = 47;
	}
	else 
	{
		startPos.y += 19;
		endPos.y += 19;
		numLinks = 100;
	}

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);

	start->setLayer(1);
	start->setLayerMask(layerMask);

	start->GetRenderObject()->SetColour(brown);
	

	GameObject* end = AddCubeToWorld(endPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	end->setLayer(1);
	end->setLayerMask(layerMask);

	end->GetRenderObject()->SetColour(brown);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		
		block->setLayer(1);
		block->setLayerMask(layerMask);
		
		block->GetRenderObject()->SetColour(brown);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("SPHERE");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("CUBE");

	AABBVolume* volume = new AABBVolume(dimensions);

	//SphereVolume* volume = new SphereVolume(dimensions.x);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position)
{
	float size			= 1.0f;
	float inverseMass	= 0.1f;

	goose = new Player(playerID);

	Vector3 offSet(5, 0, 5);

	goose->setCamera(world->GetMainCamera());

	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size,size,size) );

	if (playerID == 1000)
		goose->GetTransform().SetWorldPosition(position + offSet);
	else 
		goose->GetTransform().SetWorldPosition(position - offSet);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	if (playerID == 1000)
	{
		goose->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	}
	else
	{
		goose->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	}

	goose->SetNetworkObject(new NetworkObject(*goose, playerID));

	world->AddGameObject(goose);

	return goose;
}

GameObject* TutorialGame::AddPlayerTwoToWorld(const Vector3& position)
{
	float inverseMass = 0.1f;

	playerTwo->SetRenderObject(new RenderObject(&playerTwo->GetTransform(), gooseMesh, nullptr, basicShader));
	playerTwo->SetPhysicsObject(new PhysicsObject(&playerTwo->GetTransform(), playerTwo->GetBoundingVolume()));

	playerTwo->GetPhysicsObject()->SetInverseMass(inverseMass);
	playerTwo->GetPhysicsObject()->InitSphereInertia();

	int id;
	if (playerID == 1000)
	{
		id = 2000;
		playerTwo->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	}
	else 
	{
		id = 1000;
		playerTwo->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	}
		
	playerTwo->SetNetworkObject(new NetworkObject(*playerTwo, id));

	world->AddGameObject(playerTwo);

	return playerTwo;
}

Enemy* TutorialGame::AddParkKeeperToWorld(const Vector3& position)
{
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	Enemy* keeper = new Enemy(position, world, isServer);

	keeper->setPlayer(goose);
	keeper->setPlayerTwo(playerTwo);
	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(keeper);

	return keeper;
	
}

GameObject* TutorialGame::AddCharacterToWorld(const Vector3& position) {
	float meshSize = 4.0f;
	float inverseMass = 0.5f;

	auto pos = keeperMesh->GetPositionData();

	Vector3 minVal = pos[0];
	Vector3 maxVal = pos[0];

	for (auto& i : pos) {
		maxVal.y = max(maxVal.y, i.y);
		minVal.y = min(minVal.y, i.y);
	}

	GameObject* character = new GameObject("CHARACTER");

	float r = rand() / (float)RAND_MAX;


	AABBVolume* volume = new AABBVolume(Vector3(0.3, 0.9f, 0.3) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	character->GetTransform().SetWorldPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), r > 0.5f ? charA : charB, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddAppleToWorld(const Vector3& position) {
	Collectable* apple = new Collectable(10, position);

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4, 4, 4));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(100.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddBonusItemToWorld(const Vector3& position) {
	Collectable* box = new Collectable(20, position);

	AABBVolume* volume = new AABBVolume(Vector3(0.8, 0.8, 0.8));
	box->SetBoundingVolume((CollisionVolume*)volume);
	box->GetTransform().SetWorldScale(Vector3(0.8, 0.8, 0.8));
	box->GetTransform().SetWorldPosition(position);

	box->SetRenderObject(new RenderObject(&box->GetTransform(), cubeMesh, nullptr, basicShader));
	box->SetPhysicsObject(new PhysicsObject(&box->GetTransform(), box->GetBoundingVolume()));
	box->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	box->GetPhysicsObject()->SetInverseMass(100.0f);
	box->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(box);

	return box;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::SimpleGJKTest() {
	Vector3 dimensions		= Vector3(5, 5, 5);
	Vector3 floorDimensions = Vector3(100, 2, 100);

	GameObject* fallingCube = AddCubeToWorld(Vector3(0, 20, 0), dimensions, 10.0f);
	GameObject* newFloor	= AddCubeToWorld(Vector3(0, 0, 0), floorDimensions, 0.0f);

	delete fallingCube->GetBoundingVolume();
	delete newFloor->GetBoundingVolume();

	fallingCube->SetBoundingVolume((CollisionVolume*)new OBBVolume(dimensions));
	newFloor->SetBoundingVolume((CollisionVolume*)new OBBVolume(floorDimensions));

}

