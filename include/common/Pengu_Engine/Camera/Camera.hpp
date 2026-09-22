/**
 * @file Camera.hpp
 * @brief 3D First-Person/Free-fly camera system.
 */

#ifndef CAMERA_HPP
#define CAMERA_HPP 1

#include "GL/glew.h"
#include "Pengu_Engine/Inputs/Input.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

 /**
  * @enum CamMovement
  * @brief Directions for relative camera movement.
  */
enum CamMovement {
	kForward,
	kBackward,
	kLeft,
	kRight
};

// Default camera constants
/** @brief Default horizontal rotation (facing -Z). */
const float YAW = -90.0f;
/** @brief Default vertical rotation. */
const float PITCH = 0.0f;
/** @brief Default movement speed. */
const float SPEED = 0.01f;
/** @brief Default mouse look sensitivity. */
const float SENSITIVITY = 0.01f;
/** @brief Default Field of View (FOV). */
const float ZOOM = 45.0f;

/**
 * @class Camera
 * @brief Processes input and calculates the corresponding View Matrix for OpenGL.
 */
class Camera {

public:

	/**
	 * @brief Default constructor for Camera.
	 */
	Camera()
	{
		updateCameraVectors();
	}

	/**
	 * @brief Default destructor for Camera.
	 */
	~Camera() = default;

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	Camera(Camera&&) = default;
	Camera& operator=(Camera&&) = default;

	/**
	 * @brief Calculates the view matrix using the LookAt algorithm.
	 * @return glm::mat4 The transformation matrix representing the camera's view.
	 */
	glm::mat4 getViewMatrix() const;

	/**
	 * @brief Gets the current camera position.
	 * @return glm::vec3 The camera position.
	 */
	glm::vec3 getPosition() const { return position_; };

	/**
	 * @brief Gets the near plane distance.
	 * @return float Near plane distance.
	 */
	float getNear() const { return m_presp_near; };

	/**
	 * @brief Gets the far plane distance.
	 * @return float Far plane distance.
	 */
	float getFar() const { return m_presp_far; };

	/**
	 * @brief Creates the perspective projection matrix.
	 * @param screen_width Width of the screen.
	 * @param screen_height Height of the screen.
	 */
	void CreatePerspective(int screen_width, int screen_height);

	/**
	 * @brief Processes keyboard-style movement input.
	 * @param input Reference to the Input system.
	 * @param deltaTime Time elapsed since last frame.
	 */
	void processInput(Input& input, float deltaTime);

	/**
	 * @brief Processes mouse movement for camera rotation.
	 * @param xoffset Horizontal mouse displacement.
	 * @param yoffset Vertical mouse displacement.
	 * @param constraintPitch If true, prevents the camera from flipping over.
	 */
	void processMouseMovement(float xoffset, float yoffset, GLboolean constraintPitch = true);

	/**
	 * @brief Processes scroll wheel input to adjust zoom (FOV).
	 * @param yoffset The scroll displacement.
	 */
	void processMouseScroll(float yoffset);

	// --- Public Members ---

	/** @brief Camera position in world space. */
	glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 3.0f);
	/** @brief Camera front vector. */
	glm::vec3 front_ = glm::vec3(0.0f, 0.0f, -1.0f);
	/** @brief Camera up vector. */
	glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	/** @brief Camera right vector. */
	glm::vec3 right_ = glm::vec3(1.0f, 0.0f, 0.0f);
	/** @brief World up vector. */
	glm::vec3 worldUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
	/** @brief Current projection matrix. */
	glm::mat4 projection_;

	/**
	 * @brief Sets the movement speed.
	 * @param speed The new speed value.
	 */
	void Set_CamSpeed(float speed) { movSpeed_ = glm::clamp(speed, 0.0f, 100.0f); };
	/**
	 * @brief Gets the current movement speed.
	 * @return float The movement speed.
	 */
	float Get_CamSpeed() const { return movSpeed_; };

	/**
	 * @brief Sets the look sensitivity.
	 * @param sens The new sensitivity value.
	 */
	void Set_CamSens(float sens) { mouseSens_ = glm::clamp(sens, 0.0f, 0.5f); };
	/**
	 * @brief Gets the current look sensitivity.
	 * @return float The look sensitivity.
	 */
	float Get_CamSens() const { return mouseSens_; };

	/** @brief Horizontal Euler angle (in degrees). */
	float yaw_ = YAW;
	/** @brief Vertical Euler angle (in degrees). */
	float pitch_ = PITCH;
	/** @brief Current speed multiplier for movement. */
	float movSpeed_ = SPEED;
	/** @brief Multiplier for mouse rotation speed. */
	float mouseSens_ = SENSITIVITY;
	/** @brief Current Field of View (FOV). */
	float zoom_ = ZOOM;

	/** @brief Aspect ratio of the viewport. */
	float m_aspect;

private:
	/**
	 * @brief Re-calculates the Front, Right, and Up vectors.
	 */
	void updateCameraVectors();

	/** @brief Perspective radius (zoom). */
	float m_presp_radius;
	/** @brief Screen width for perspective calculation. */
	float m_presp_scr_width;
	/** @brief Screen height for perspective calculation. */
	float m_presp_scr_height;
	/** @brief Near plane distance for perspective calculation. */
	float m_presp_near = 0.1f;
	/** @brief Far plane distance for perspective calculation. */
	float m_presp_far = 2000.0f;
};

#endif // !CAMERA_HPP