#include "Pengu_Engine/Camera/Camera.hpp"
#include "Pengu_Engine/Inputs/Input.hpp"


glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::CreatePerspective(int screen_width, int screen_height)
{
	if (screen_width > 0) {
		m_presp_radius = zoom_;
		m_presp_scr_width = static_cast<float>(screen_width);
		m_presp_scr_height = static_cast<float>(screen_height);

		m_aspect = static_cast<float>(m_presp_scr_width) / static_cast<float>(m_presp_scr_height);

		projection_ = glm::perspective(glm::radians(m_presp_radius), m_aspect, m_presp_near, m_presp_far);
	}

}

void Camera::processInput(Input& input, float deltaTime)
{
	float velocity = movSpeed_ * deltaTime;
	if (input.isDown(Action::Up))
		position_ += front_ * velocity;
	if (input.isDown(Action::Down))
		position_ -= front_ * velocity;
	if (input.isDown(Action::Left))
		position_ -= right_ * velocity;
	if (input.isDown(Action::Right))
		position_ += right_ * velocity;
	if (input.isDown(Action::Q))
		position_ -= worldUp_ * velocity;
	if (input.isDown(Action::E))
		position_ += worldUp_ * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constraintPitch)
{
	xoffset *= mouseSens_;
	yoffset *= mouseSens_;

	yaw_ += xoffset;
	pitch_ -= yoffset;

	if (constraintPitch) {
		if (pitch_ > 89.0f)
			pitch_ = 89.0f;
		if (pitch_ < -89.0f)
			pitch_ = -89.0f;
	}

	updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
	zoom_ -= yoffset;
	if (zoom_ < 1.0f)
		zoom_ = 1.0f;
	if (zoom_ > 179.0f)
		zoom_ = 179.0f;
}

void Camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front.y = sin(glm::radians(pitch_));
	front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));

	front_ = glm::normalize(front);

	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));
}
