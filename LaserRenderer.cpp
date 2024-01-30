//
// Created by jdhyd on 1/29/2024.
//

#include "LaserRenderer.h"
#include "OGLRenderer.h"

LaserRenderer::LaserRenderer(GLuint tex) {
    texture = tex;
}

void LaserRenderer::InitLaser(GLuint hdrBuf) {
    hdrBuffer = hdrBuf;
    quad = std::unique_ptr<Mesh>(Mesh::GenerateQuad());
    shader = std::make_unique<Shader>(SHADERPATH "QuadVert.glsl", SHADERPATH "LaserFrag.glsl");
}

void LaserRenderer::DrawLaser(OGLRenderer& renderer, GLuint screenTexture) {
    glBindFramebuffer(GL_FRAMEBUFFER, hdrBuffer);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    renderer.BindShader(shader.get());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    quad->Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint LaserRenderer::GetTexture() {
    return texture;
}



