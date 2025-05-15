#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up; // Camera local UP vector
    glm::vec3 WorldUp; // World UP vector

    GLfloat Yaw = -90.0f; // Start looking along -Z
    GLfloat Pitch = 0.0f;
    GLfloat Roll = 0.0f; // Not used for now, but included for completeness

    // Camera options
    GLfloat MovementSpeed = 5.01f;
    GLfloat MouseSensitivity = 0.08f;
    GLfloat JumpForce = 5.0f;
    GLfloat Gravity = -9.81f;

    float playerHeight = 0.2f; // Height of the player (for jumping)
    float playerRadius = 0.2f;
    bool isGrounded = false;

    Camera(glm::vec3 position) : Position(position), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
    {
        //this->Up = glm::vec3(0.0f, 1.0f, 0.0f); // World up
        //this->updateCameraVectors();
        Velocity = glm::vec3(0.0f);
        updateCameraVectors();
    }


    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }


    glm::vec3 ProcessInput(GLFWwindow *window, GLfloat deltaTime)
    {
        glm::vec3 inputDirection(0.0f);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            inputDirection += glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            inputDirection -= glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            inputDirection -= Right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            inputDirection += Right;
        
        if (glm::length(inputDirection) > 0.0f)
            inputDirection = glm::normalize(inputDirection) * MovementSpeed;

        // Jump only if grounded
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isGrounded) {
            Velocity.y = JumpForce;
            isGrounded = false;
        }

        // Apply movement speed only if there's input
        //if (glm::length(inputDirection) > 0.0f) {
        //    inputDirection = glm::normalize(inputDirection) * MovementSpeed;
        //}

        // Apply gravity
        Velocity.y += Gravity * deltaTime;
        
        // Update position based on velocity
        glm::vec3 movement = inputDirection * deltaTime;
        movement.y = Velocity.y * deltaTime;
        
        return movement;
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