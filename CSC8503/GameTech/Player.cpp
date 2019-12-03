#include "Player.h"

using namespace NCL;
using namespace CSC8503;

Player::Player() : GameObject("PLAYER") 
{
	speed = 50;
	camOffset = Vector3(0, 10, -10);
	yaw = 0.0f;
	mainCamera = nullptr;
	jumpPower = 100;
}

Player::~Player() 
{

}

void Player::UpdatePlayer(float dt)
{
	UpdateKeys(dt);
	UpdateCamera(dt);
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

void Player::UpdateKeys(float dt)
{
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	transform.SetLocalOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));

	//mainCamera->SetYaw(yaw);

	Vector4 z = transform.GetWorldMatrix().GetColumn(2);

	Vector3 forward = Vector3(z.x, z.y, z.z);

	Vector4 x = transform.GetWorldMatrix().GetColumn(0);

	Vector3 right = Vector3(x.x, x.y, x.z);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		physicsObject->AddForce(forward * speed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		physicsObject->AddForce(-forward * speed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		physicsObject->AddForce(right * speed);

	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		physicsObject->AddForce(-right * speed);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
		physicsObject->AddForce(Vector3(0, 1, 0) * jumpPower);
	}
}