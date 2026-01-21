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
	glm::vec3 view = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(view, up));
	
	if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT)) {
		float angle = ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 yaw = glm::rotate(
			glm::mat4(1.f),
			glm::radians(angle),
			glm::vec3(0.f, 1.f, 0.f)
		);
		glm::vec3 rotatedView = yaw * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}
	else if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_RIGHT)) {
		float angle = -ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 yaw = glm::rotate(
			glm::mat4(1.f),
			glm::radians(angle),
			glm::vec3(0.f, 1.f, 0.f)
		);
		glm::vec3 rotatedView = yaw * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}
	else if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_UP)) {
		float angle = ROTATE_SPEED * static_cast<float>(dt);
		glm::mat4 pitch = glm::rotate(
			glm::mat4(1.f),
			glm::radians(angle),
			right
		);
		glm::vec3 rotatedView = pitch * glm::vec4(view, 0.f);
		target = position + rotatedView;
		isDirty = true;
	}

	// Mouse yaw (left/right) - unlimited rotation
	double deltaX = MouseController::GetInstance()->GetMouseDeltaX();
	if (deltaX != 0.0) {
		float yawAngle = -deltaX * ROTATE_SPEED * 0.1 * static_cast<float>(dt);
		glm::mat4 yaw = glm::rotate(
			glm::mat4(1.f),
			glm::radians(yawAngle),
			glm::vec3(0.f, 1.f, 0.f)
		);
		view = yaw * glm::vec4(view, 0.f);
		isDirty = true;
	}

	// Mouse pitch (up/down) - unlimited rotation
	double deltaY = MouseController::GetInstance()->GetMouseDeltaY();
	if (deltaY != 0.0) {
		float pitchAngle = deltaY * ROTATE_SPEED * 0.1 * static_cast<float>(dt);
		
		// Recalculate right vector after yaw
		right = glm::normalize(glm::cross(view, glm::vec3(0.f, 1.f, 0.f)));
		
		glm::mat4 pitch = glm::rotate(
			glm::mat4(1.f),
			glm::radians(pitchAngle),
			right
		);
		view = pitch * glm::vec4(view, 0.f);
		isDirty = true;
	}
	
	// Update target with the new view direction
	if (deltaX != 0.0 || deltaY != 0.0) {
		target = position + glm::normalize(view);
	}
}

void FPCamera::Refresh()
{
	if (!this->isDirty) return;
	glm::vec3 view = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(view, glm::vec3(0.f, 1.f, 0.f)));

	// Recalculate the up vector
	this->up = glm::normalize(glm::cross(right, view));

	this->isDirty = false;
}