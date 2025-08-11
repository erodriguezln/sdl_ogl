#ifndef SDL_OGL_CAMERA_H
#define SDL_OGL_CAMERA_H

#include <glm.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float fov;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    ~Camera();

    glm::mat4 getViewMatrix() const;

    void processKeyboard(Camera_Movement direction, float deltaTime);

    void processMouse(float xOffset, float yOffset);

private:
    void updateCameraVectors();
};


#endif //SDL_OGL_CAMERA_H