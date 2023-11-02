//
// Created by c3042750 on 02/11/2023.
//

#include "Cubemap.h"

float Cubemap::SkyboxVerts[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

Cubemap::Cubemap() {
    stbi_set_flip_vertically_on_load(false);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    std::vector<const char*> texturePaths = {
            TEXTUREPATH "cubemap/right.png",
            TEXTUREPATH "cubemap/left.png",
            TEXTUREPATH "cubemap/top.png",
            TEXTUREPATH "cubemap/bottom.png",
            TEXTUREPATH "cubemap/front.png",
            TEXTUREPATH "cubemap/back.png"
    };

    int width, height, channels;
    unsigned char *data;
    for (unsigned int i = 0; i < texturePaths.size(); i++) {
        data = stbi_load(texturePaths[i], &width, &height, &channels, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    shader = new Shader(SHADERPATH "CubemapVert.glsl", SHADERPATH "CubemapFrag.glsl");

    if (!shader->LoadSuccess()) {
        std::cout << "Cubemap Shader not loaded :(" << std::endl;
        return;
    }

    glGenVertexArrays(1, &arrayObj);
    glBindVertexArray(arrayObj);
    glGenBuffers(1, &bufferId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, 108 * sizeof(float), &SkyboxVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_BUFFER);

    glObjectLabel(GL_BUFFER, arrayObj, -1, "Positions");

    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(true);
}

Cubemap::~Cubemap() {
    glDeleteVertexArrays(1, &arrayObj);
    glDeleteTextures(1, &textureId);
}

void Cubemap::Draw(OGLRenderer *context) {
    glDepthMask(GL_FALSE);
    context->BindShader(shader);
    context->UpdateShaderMatrices();

    Matrix4 viewAdj = Matrix4(context->viewMatrix);
    viewAdj.values[12] = 0.0f;
    viewAdj.values[13] = 0.0f;
    viewAdj.values[14] = 0.0f;

    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "viewMatrix"), 1, false, viewAdj.values);
    glBindVertexArray(arrayObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}
