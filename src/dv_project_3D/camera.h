#ifndef CAMERA_H
#define CAMERA_H

#include "libs.h"

using namespace OGLAppFramework;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.4f;
const float ZOOM = 45.0f;

class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 dPosition;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	float width, height, depth;
	bool onGround;
	bool isNearWalls;
	Camera_Movement lastButtonPressed;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		onGround = false;
		isNearWalls = false;
		width = 0.5; height = 0.5; depth = 1;
		Position = position;
		dPosition = glm::vec3(0, 0, 0);
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		dPosition = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		glm::vec3 rotationVec = glm::vec3(1, 0, 1);
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			if (!isNearWalls || direction != lastButtonPressed) {  dPosition += rotationVec * Front * velocity;};
		if (direction == BACKWARD)
			if (!isNearWalls || direction != lastButtonPressed) {  dPosition -= rotationVec * Front * velocity;};
		if (direction == LEFT)
			if (!isNearWalls || direction != lastButtonPressed) {  dPosition -= Right * velocity; };
		if (direction == RIGHT)
			if (!isNearWalls || direction != lastButtonPressed) {  dPosition += Right * velocity;};
		if (direction == UP) {
			if (onGround) { onGround = false; dPosition.y = 0.3; };
		}
		lastButtonPressed = direction;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, gl::GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	glm::vec3 getBeforePlayerPosition() {
		return glm::vec3(Position.x + .5, Position.y + .5, Position.z + .5) + Front;
	}

	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

	void update(float deltaTime, bool mass[100][100][100]) {
		float velocity = MovementSpeed * deltaTime * 0.1;

		if (!onGround) dPosition.y -= velocity;
		onGround = false;
		isNearWalls = false;

		Position.x += dPosition.x;
		Position.y += dPosition.y;
		Position.z += dPosition.z;
		collision(mass);

		dPosition.x = dPosition.z = 0;
		if (Position.y < -100.f) {
			Position.y = 20.f;
			dPosition.y = 0;
		}
	}

private:
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void collision(bool mass[100][100][100]) {
		if (check(Position.x, Position.y, Position.z)) {
			if (mass[(int)(Position.x + .5)][int(Position.y - .5)][(int)(Position.z + .5)]) {
				onGround = true; dPosition.y = 0;
			}
			if (mass[(int)(Position.x + .5)][int(Position.y + .5)][(int)(Position.z + .5)]) {
				isNearWalls = true; dPosition.x = 0; dPosition.z = 0; 
			}
		}
	}

	bool check(int x, int y, int z)
	{
		if ((x < 0) || (x >= 100) ||
			(y < 0) || (y >= 100) ||
			(z < 0) || (z >= 100)) return false;

		return true;
	}
};
#endif