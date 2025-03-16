#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up; // Camera local UP vector

    GLfloat Yaw = -90.0f; // Start looking along -Z
    GLfloat Pitch = 0.0f;
    GLfloat Roll = 0.0f; // Not used for now, but included for completeness

    // Camera options
    GLfloat MovementSpeed = 0.01f;
    GLfloat MouseSensitivity = 0.01f;

    Camera(glm::vec3 position) : Position(position)
    {
        this->Up = glm::vec3(0.0f, 1.0f, 0.0f); // World up
        this->updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    glm::vec3 ProcessInput(GLFWwindow *window, GLfloat deltaTime)
    {
        glm::vec3 direction{0};

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            direction += Front; // Forward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            direction -= Front; // Backward
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            direction -= Right; // Left
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            direction += Right; // Right
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            direction += Up; // Up
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            direction -= Up; // Down

        // Normalize direction only if non-zero to avoid division by zero
        if (glm::length(direction) > 0.0f)
        {
            direction = glm::normalize(direction) * MovementSpeed * deltaTime;
        }
        return direction;
    }

    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = GL_TRUE)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw += xoffset;
        this->Pitch -= yoffset;

        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        this->updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));

        this->Front = glm::normalize(front);
        this->Right = glm::normalize(glm::cross(this->Front, glm::vec3(0.0f, 1.0f, 0.0f))); // World up
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
    }
};