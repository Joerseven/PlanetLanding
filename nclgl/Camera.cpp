//
// Created by jdhyd on 10/24/2023.
//

#include "Camera.h"
#include "Window.h"
#include <algorithm>


void Camera::UpdateCamera(float dt) {

}

void Camera::UpdateLookDirection(const Vector2 &direction) {
    Pitch -= (direction.y);
    Yaw -= (direction.x);

    Pitch = std::min(Pitch, 90.0f);
    Pitch = std::max(Pitch, -90.0f);

    if (Yaw < 0) {
        Yaw += 360.f;
    }

    if (Yaw > 360.f) {
        Yaw -= 360.f;
    }

}

void Camera::UpdatePosition(const Vector3& direction, float speed) {
    Matrix4 rotation = Matrix4::Rotation(Yaw, Vector3(0, 1, 0));
    Position += rotation * direction * speed;
}

Matrix4 Camera::BuildViewMatrix() const {
    return Matrix4::Rotation(-Pitch, Vector3(1, 0, 0))
        * Matrix4::Rotation(-Yaw, Vector3(0, 1, 0))
        * Matrix4::Translation(-Position);
}



