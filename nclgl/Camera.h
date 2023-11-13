//
// Created by jdhyd on 10/24/2023.
//

#ifndef SECONDMATRIX_CAMERA_H
#define SECONDMATRIX_CAMERA_H

#include "Vector3.h"
#include "Matrix4.h"
#include "Vector2.h"


class Camera {
public:

    float Yaw, Pitch;
    Vector3 Position;

    Camera() {
        Yaw = 0.0f;
        Pitch = 0.0f;
    }

    Camera(float pitch, float yaw, Vector3 position) {
        Pitch = pitch;
        Yaw = yaw;
        Position = position;
    }

    ~Camera() = default;

    void UpdateCamera(float dt = 1.0f);

    Matrix4 BuildViewMatrix() const;

    void UpdatePosition(const Vector3& direction, float speed);
    void UpdateLookDirection(const Vector2& direction);
};


#endif //SECONDMATRIX_CAMERA_H
