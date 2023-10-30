//
// Created by jdhyd on 10/24/2023.
//

#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
    Pitch -= (Window::GetMouse()->GetRelativePosition().y);
    Yaw -= (Window::GetMouse()->GetRelativePosition().x);

    Pitch = std::min(Pitch, 90.0f);
    Pitch = std::max(Pitch, -90.0f);

    if (Yaw < 0) {
        Yaw += 360.f;
    }

    if (Yaw > 360.f) {
        Yaw -= 360.f;
    }

    Matrix4 rotation = Matrix4::Rotation(Yaw, Vector3(0, 1, 0));

    Vector3 forward = rotation * Vector3(0, 0, -1);
    Vector3 right = rotation * Vector3(1, 0, 0);

    float speed = 600.f * dt;

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
        Position += forward * speed;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
        Position -= forward * speed;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
        Position -= right * speed;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
        Position += right * speed;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
        Position.y += speed;
    }

    if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
        Position.y -= speed;
    }


}

Matrix4 Camera::BuildViewMatrix() const {
    return Matrix4::Rotation(-Pitch, Vector3(1, 0, 0))
        * Matrix4::Rotation(-Yaw, Vector3(0, 1, 0))
        * Matrix4::Translation(-Position);
}

