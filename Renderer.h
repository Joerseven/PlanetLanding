//
// Created by jdhyd on 10/30/2023.
//

#ifndef PLANETLANDING_RENDERER_H
#define PLANETLANDING_RENDERER_H

#include "OGLRenderer.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Cubemap.h"
#include "Component.h"
#include "Light.h"
#include <queue>
#include "SimplexNoise.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include "Quaternion.h"

#define SHADERPATH "../shaders/"

class Renderer;

struct iVector2 {
    int x;
    int y;
};

struct bloomMip {
    Vector2 size;
    iVector2 iSize;
    GLuint texture;
};

class bloomFBO {
public:
    bloomFBO() : mInit(false) {};
    ~bloomFBO() = default;
    bool Init(GLuint windowWidth, GLuint windowHeight, GLuint mipNumber);
    void Destroy();
    void BindForWriting();
    const std::vector<bloomMip>& MipChain() const;
private:
    unsigned int mFBO;
    std::vector<bloomMip> mMipChain;
    bool mInit;
};

class BloomRenderer {
public:
    BloomRenderer(int windowWidth, int windowHeight);
    ~BloomRenderer() = default;
    void Destroy();
    void RenderBloomTexture(unsigned int srcTexture, float filterRadius, Renderer& context);
    unsigned int BloomTexture();
    unsigned int FinalTexture();
private:
    void RenderDownsamples(unsigned int srcTexture, Renderer& context);
    void RenderUpsamples(float filterRadius, Renderer& context);
    void Prefilter(unsigned int srcTexture, Renderer &context);
    void PostFilter(unsigned int srcTexture, unsigned int bloomTexture, Renderer &context);


    bloomFBO mFBO;
    iVector2 mSrcViewportSize;
    Vector2 mSrcViewportSizeFloat;
    Shader* mDownsampleShader;
    Shader* mUpsampleShader;
    Shader* mPrefilterShader;
    Shader* mPostfilterShader;
    GLuint mPrefilterTexture;
    GLuint mPostfilterTexture;
    Mesh* quad;
};

struct CameraTrack {
    Vector3 points[4];
    float duration;
    float elapsed;
};

class Model {
public:
    Model() = default;
    ~Model() = default;
    Mesh* mesh;
    GLuint texture;

    Matrix4 localTransform;
    Shader* shader;
    Light* light;

    void Draw(Renderer *context);

};

class Renderer: public OGLRenderer {
public:
    Renderer(Window& parent);
    ~Renderer();

    void RenderScene() override;
    void UpdateScene(float dt) override;
public:
    Shader *planetShader, *sunShader;
    Camera* camera;
    Mesh* finalQuad;
    Light* light;
    Cubemap* cubemap;
    Shader* hdrShader;
    Shader* atmosphereShader;
    Shader* shipShader;
    Shader* antiShader;
    std::unique_ptr<Mesh> shipModel;
    GLuint colorBuffer;
    GLuint depthTexture;
    GLuint hdrFramebuffer;
    Matrix4 shipTransform;
    Vector3 shipDirection;
    GLuint antiATex;
    GLuint antiABuffer;
    Quaternion shipRotation;

    float pitching = 0.0f;
    float yawing = 0.0f;

    float shipSpeed = 0;
    float shipAcceleration = 2;

    GLuint atmosphereFramebuffer;
    GLuint atmosphereTexture;

    Registry registry;
    Noise* noise;
    BloomRenderer* bloomRenderer;
    std::queue<CameraTrack> cameraQueue;

    void RenderSceneToBuffer();

    void RenderTextureToScreen(GLuint texture);

    void UpdateCameraMovement(float dt);

    void TranslateCamera(float dt);

    void UpdateLookDirection(float dt) const;

    void UpdateShip(float dt);

    void AddCameraAnimation(CameraTrack &&track);

    bool MovePosition(float dt, Vector3& position);

    void DrawModels();

    void RenderPlanetAtmosphere(GLuint tex, GLuint depth);

    void DrawShip();

    Entity RegisterPlanet(Transform transform, Vector4 color, Shader *shader, GLuint texture);

    GLuint CreatePostPassTexture();

    GLuint CreateDepthTexture();

    GLuint CreateHdrFramebuffer(GLuint colorBuffer, GLuint depthTexture);

    void RegisterComponents();

    void BuildScene();

    void LoadShaders();

    void CreateTextures();

    void InitAntiAliasing();


    void AntiAliasingPass(GLuint tex);
};


#endif //PLANETLANDING_RENDERER_H
