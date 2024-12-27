#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace gps {

// Constructor: Initialize camera parameters
Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
    this->cameraPosition = cameraPosition;
    this->cameraTarget = cameraTarget;
    this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
    this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
    this->cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
}

// Return the view matrix using glm::lookAt
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
}

// Move the camera based on direction and speed
void Camera::move(MOVE_DIRECTION direction, float speed) {
    if (direction == MOVE_FORWARD) {
        cameraPosition += speed * cameraFrontDirection;
    }
    if (direction == MOVE_BACKWARD) {
        cameraPosition -= speed * cameraFrontDirection;
    }
    if (direction == MOVE_RIGHT) {
        cameraPosition += speed * cameraRightDirection;
    }
    if (direction == MOVE_LEFT) {
        cameraPosition -= speed * cameraRightDirection;
    }
}

// Rotate the camera based on pitch and yaw offsets
void Camera::rotate(float pitchOffset, float yawOffset) {
    yaw += yawOffset;
    pitch += pitchOffset;
    
    // Constrain pitch to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFrontDirection = glm::normalize(direction);
    
    // Update right and up directions
    cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
    void Camera::setPosition(const glm::vec3& newPos)
    {
        this->cameraPosition = newPos;
        // Optionally update your target if you want the same orientation
        // But typically you do that in setLookDirection()
    }

    void Camera::setLookDirection(const glm::vec3& direction)
    {
        // For a simple camera, we often do "target = position + direction"
        // so the camera looks in 'direction' from 'position'.
        this->cameraTarget = this->cameraPosition + cameraFrontDirection;

        // If your camera uses yaw/pitch angles, you'd need to recalc them from 'direction'
        // or store 'direction' in some local variable.
        // e.g. you might do something like:
        //
        // this->frontDirection = glm::normalize(direction);
        // recalc yaw/pitch from frontDirection if needed
    }
// Get the current camera position
glm::vec3 Camera::getPosition() const {
    return cameraPosition;
}
}
