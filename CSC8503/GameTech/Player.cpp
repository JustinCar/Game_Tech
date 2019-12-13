#include "Player.h"

using namespace NCL;
using namespace CSC8503;

Player::Player(int id) : GameObject("PLAYER")
{
	speed = 500;
	camOffset = Vector3(0, 15, -10);
	yaw = 0.0f;
	mainCamera = nullptr;
	jumpPower = 10000;
	jumpCoolDown = 2;
	jumpTimer = 2;

	playerID = id;

	swimPower = 10000;
	swimTimer = 1;
	swimCoolDown = 1;
	isSwimming = false;

	for (int i = 0; i < 6; i++)
		buttonStates[i] = false;

	layer = 2;
	layerMask = 0; // Collide with everything
}

Player::~Player() 
{

}

void Player::Update(float dt)
{
	if (playerID == 1000)
		UpdateServerPlayerKeys(dt);
	else
		UpdateClientPlayerKeys(dt);

	UpdateCamera(dt);
}

void Player::Trigger(GameObject& obj)
{

}

void Player::UpdateCamera(float dt)
{
	Vector3 pos = transform.GetWorldPosition();

	Vector4 f = transform.GetWorldMatrix().GetColumn(2);

	Vector3 forward = Vector3(f.x, f.y, f.z);

	Vector3 camPos = pos;

	camPos -= forward * 10;
	camPos.y += 2;

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, transform.GetWorldPosition(), Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	mainCamera->SetPosition(camPos);
	//mainCamera->SetPitch(angles.x);
	mainCamera->SetYaw(angles.y);
}

void Player::UpdateServerPlayerKeys(float dt)
{
	yaw -= (Window::GetMouse()->GetRelativePosition().x);
	jumpTimer -= dt;

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	transform.SetLocalOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));

	Vector4 z = transform.GetWorldMatrix().GetColumn(2);

	Vector3 forward = Vector3(z.x, z.y, z.z);

	Vector4 x = transform.GetWorldMatrix().GetColumn(0);

	Vector3 right = Vector3(x.x, x.y, x.z);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE) && jumpTimer <= 0) {
		jumpTimer = jumpCoolDown;
		physicsObject->AddForce(Vector3(0, 1, 0) * jumpPower);
	}

	if (isSwimming)
	{
		swimTimer -= dt;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W) && swimTimer <= 0) {
			swimTimer = swimCoolDown;
			physicsObject->AddForce(forward * swimPower);
		}
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W) && !isSwimming) {
		physicsObject->AddForce(forward * speed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S) && !isSwimming) {
		physicsObject->AddForce(-forward * speed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A) && !isSwimming) {
		physicsObject->AddForce(right * speed);

	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D) && !isSwimming) {
		physicsObject->AddForce(-right * speed);
	}
}

void Player::UpdateClientPlayerKeys(float dt)
{
	yaw -= (Window::GetMouse()->GetRelativePosition().x);
	jumpTimer -= dt;

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	transform.SetLocalOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));

	Vector4 z = transform.GetWorldMatrix().GetColumn(2);

	Vector3 forward = Vector3(z.x, z.y, z.z);

	Vector4 x = transform.GetWorldMatrix().GetColumn(0);

	Vector3 right = Vector3(x.x, x.y, x.z);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RETURN) && jumpTimer <= 0) {
		jumpTimer = jumpCoolDown;
		buttonStates[4] = true;
	}
	else
	{
		buttonStates[4] = false;
	}

	buttonStates[5] = isSwimming;

	if (isSwimming)
	{
		swimTimer -= dt;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::U) && swimTimer <= 0) {
			swimTimer = swimCoolDown;
			buttonStates[0] = true;
		}
		else
		{
			buttonStates[0] = false;
		}
	}
	else
	{
		buttonStates[0] = false;
	}
	

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::U) && !isSwimming) {
		buttonStates[0] = true;
	}
	else if (!isSwimming)
	{
		buttonStates[0] = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::J) && !isSwimming) {
		buttonStates[1] = true;
	}
	else
	{
		buttonStates[1] = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::H) && !isSwimming) {
		buttonStates[2] = true;
	}
	else
	{
		buttonStates[2] = false;
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::K) && !isSwimming) {
		buttonStates[3] = true;
	}
	else
	{
		buttonStates[3] = false;
	}
}

void Player::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->getLayer() == 6)
		isSwimming = true;
}

void Player::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->getLayer() == 6)
		isSwimming = false;
}