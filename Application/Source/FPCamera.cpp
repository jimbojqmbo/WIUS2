#include "FPCamera.h"
#include "KeyboardController.h"

//Include GLFW
#include <GLFW/glfw3.h>

#include "MouseController.h"

FPCamera::FPCamera() : isDirty(false)
{
	this->position = glm::vec3(0, 0, 0);
	this->target = glm::vec3(0, 0, 0);
	this->up = glm::vec3(0, 1, 0);
}

FPCamera::~FPCamera()
{
}

void FPCamera::Init(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
	this->position = position;
	this->target = target;
	this->up = up;
	this->isDirty = true;

	Refresh();
}

void FPCamera::Reset()
{
}

void FPCamera::Update(double dt)
{
	static const float ROTATE_SPEED = 100.0f;
	static const float ZOOM_SPEED = 10.0f;
	glm::vec3 view = glm::normalize(target - position); // calculate the new view vector
	glm::vec3 right = glm::normalize(glm::cross(view, up));
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT)) {
		// Calculate amount of angle to rotate
		float angle = ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 yaw = glm::rotate(
			glm::mat4(1.f), // Default identity
			glm::radians(angle), // Convert degree angle to radians
			glm::vec3(up.x, up.y, up.z) // Use camera Up vector to rotate
		);
		// Calculate the rotated view vector
		glm::vec3 rotatedView = yaw * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}
	else if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) {
		// To do
		// Calculate amount of angle to rotate
		float angle = -ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 yaw = glm::rotate(
			glm::mat4(1.f), // Default identity
			glm::radians(angle), // Convert degree angle to radians
			glm::vec3(up.x, up.y, up.z) // Use camera Up vector to rotate
		);
		// Calculate the rotated view vector
		glm::vec3 rotatedView = yaw * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}
	else if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP)) {
		right.y = 0; // Right vector should not point any y direction
		// Calculate amount of angle to rotate
		float angle = ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 pitch = glm::rotate(
			glm::mat4(1.f), // Default identity
			glm::radians(angle), // Convert degree angle to radians
			glm::vec3(up.x, up.y, up.z)); // Rotate on right vector
		// Calculate the rotated view vector
		glm::vec3 rotatedView = pitch * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}
	//else if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP)) {
		// To do
		//this->Refresh();
	//}

	double deltaX = MouseController::GetInstance() -> GetMouseDeltaX();
	float angle = -deltaX * ROTATE_SPEED * 0.1 * static_cast<float>(dt);
	glm::mat4 yaw = glm::rotate(
	glm::mat4(1.f),// matrix to modify
	glm::radians(angle),// rotation angle in degree and converted to radians
	glm::vec3(up.x, up.y, up.z)// the axis to rotate along
	);
	glm::vec3 yawView = yaw * glm::vec4(view, 0.f);
	target = position + yawView;
	isDirty = true;
	}

void FPCamera::Refresh()
{
	if (!this->isDirty) return;
	glm::vec3 view = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(up, view));

	// Recalculate the up vector
	this->up = glm::normalize(glm::cross(view, right));

	this->isDirty = false;
}