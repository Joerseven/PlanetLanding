//
// Created by jdhyd on 10/29/2023.
//

#ifndef CHALLENGE_LIGHT_H
#define CHALLENGE_LIGHT_H

#include "Vector3.h"
#include "Vector4.h"


class Light {
public:
    Light() = default;
    Light(const Vector3& position, const Vector4& color, float radius) {
        Position = position;
        Color = color;
        Radius = radius;
    }
    ~Light() = default;

    Vector3 Position;
    float Radius;
    Vector4 Color;
};


#endif //CHALLENGE_LIGHT_H
