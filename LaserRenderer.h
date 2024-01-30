//
// Created by jdhyd on 1/29/2024.
//

#ifndef PLANETLANDING_LASERRENDERER_H
#define PLANETLANDING_LASERRENDERER_H


#include "glad/glad.h"
#include "Mesh.h"
#include "OGLRenderer.h"
#include "Shader.h"

class LaserRenderer {
public:
    LaserRenderer(GLuint tex);
    void InitLaser(GLuint hdrBuf);
    void DrawLaser(OGLRenderer& renderer, GLuint screenTexture);
    GLuint GetTexture();
private:
    GLuint texture;
    GLuint hdrBuffer;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Mesh> quad;
};


#endif //PLANETLANDING_LASERRENDERER_H
