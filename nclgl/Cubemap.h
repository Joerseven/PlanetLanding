//
// Created by c3042750 on 02/11/2023.
//

#ifndef PLANETLANDING_CUBEMAP_H
#define PLANETLANDING_CUBEMAP_H

#include "glad/glad.h"
#include "Texture.h"
#include "Shader.h"
#include "OGLRenderer.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include <string>
#include <vector>
#include <array>


class Cubemap {

public:
    Cubemap();

    ~Cubemap();

    void Draw(OGLRenderer *context);

    GLuint textureId;
protected:
    GLuint arrayObj;
    GLuint bufferId;
    Shader* shader;
    static float SkyboxVerts[108];
};


#endif //PLANETLANDING_CUBEMAP_H
