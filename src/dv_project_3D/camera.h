#ifndef CAMERA_H
#define CAMERA_H

#include "libs.h"

using namespace OGLAppFramework;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
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
	bool isFrontCollision;
	bool isBackCollision;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		onGround = false;
		isFrontCollision = false;
		isBackCollision = false;
		width = 0; height = 0; depth = 0;
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
		dPosition = glm::vec3(0, 0, 0);
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
		if (direction == FORWARD && !isFrontCollision)
			dPosition += rotationVec * Front * velocity;
		if (direction == BACKWARD && !isBackCollision)
			dPosition -= rotationVec * Front * velocity;
		if (direction == LEFT && !isBackCollision)
			dPosition -= Right * velocity;
		if (direction == RIGHT && !isFrontCollision)
			dPosition += Right * velocity;
		if (direction == UP) {
			if (onGround) { onGround = false; dPosition.y = 0.3; };
		}
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

	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

	void update(float deltaTime, bool mass[1000][1000][1000]) {
		float velocity = MovementSpeed * deltaTime * 0.1;

		if (!onGround) dPosition.y -= velocity;
		onGround = false;

		Position.x += dPosition.x;
		Position.y += dPosition.y;
		Position.z += dPosition.z;
		collision(glm::vec3(Position.x, Position.y, Position.z), mass);

		dPosition.x = dPosition.z = 0;
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

	void collision(glm::vec3 point, bool mass[1000][1000][1000])
	{
		if ((point.x > 0 || point.x <= 10) && point.y + 0.5 > 0 && (point.z > 0 || point.x <= 10)) {
			if (mass[(int)(point.x)][(int)(point.y - 0.5)][(int)(point.z)]) {
				onGround = true; dPosition.y = 0;
			}
			if (mass[(int)(point.x + 0.5)][(int)(point.y + 0.5)][(int)(point.z)]) {
				std::cout << point.x << " " << point.y << " " << point.z << std::endl;
				isFrontCollision = true;
			} else if (mass[(int)(point.x)][(int)(point.y + 0.5)][(int)(point.z + 0.5)]) {
				std::cout << point.x << " "<< point.z << std::endl;
				isBackCollision = true;
			} else {
				isBackCollision = false;
				isFrontCollision = false;
			}
		}

	}

	bool check(int x, int y, int z)
	{
		if ((x < 0) || (x >= 1000) ||
			(y < 0) || (y >= 1000) ||
			(z < 0) || (z >= 1000)) return false;

		return true;
	}
};
#endif