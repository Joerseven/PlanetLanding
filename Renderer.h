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
#include <memory>
#include <algorithm>
#include <cmath>
#include "Quaternion.h"
#include "BloomRenderer.h"

#define SHADERPATH "../shaders/"

class Renderer;

struct Segment
{
    Vector3 a;
    Vector3 b;
    Vector3 c;
    Vector3 d;
};

struct CameraTrack {
    Vector3 point;
    float yaw;
    float pitch;
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
    Shader* shipShader;
    Shader* antiShader;
    Shader* reflectiveShader;

    GLuint onePlanetTexture;


    Mesh* shipModel;
    GLuint colorBuffer;
    GLuint depthTexture;
    GLuint hdrFramebuffer;
    Matrix4 shipTransform;
    Vector3 shipDirection;
    GLuint antiATex;
    GLuint antiABuffer;
    Quaternion shipRotation;
    bool inSpace;

    Mesh* rockMesh;
    Mesh* treeModel;
    Mesh* waterQuad;

    Vector3 testVector1, testVector2;

    int currentTrack;
    float currentElapsed;
    float trackDuration;
    Segment currentSegment;

    bool autoPilot;

    float pitching = 0.0f;
    float yawing = 0.0f;

    float shipSpeed = 0;
    float shipAcceleration = 2;

    GLuint atmosphereFramebuffer;
    GLuint atmosphereTexture;

    Registry registry;
    Registry landRegistry;
    BloomRenderer* bloomRenderer;
    std::vector<CameraTrack> cameraQueue;
    std::vector<CameraTrack> spaceshipTrack;

    void RenderSceneToBuffer();

    void RenderTextureToScreen(GLuint texture);

    void UpdateCameraMovement(float dt);

    void TranslateCamera(float dt);

    void UpdateLookDirection(float dt) const;

    void DrawModels();

    Entity RegisterPlanet(const Transform& transform, const Vector4& color, Shader *shader, GLuint texture);

    GLuint CreatePostPassTexture();

    GLuint CreateDepthTexture();

    GLuint CreateHdrFramebuffer(GLuint colorBuffer, GLuint depthTexture);

    void RegisterComponents();

    void BuildScene();

    void LoadShaders();

    void CreateTextures();

    void InitAntiAliasing();


    void AntiAliasingPass(GLuint tex);

    bool MovePosition(float dt);

    void CreateCameraQueue();

    int TestSphereIntersect(const Vector3& position, float scale);

    Entity RegisterMesh(Registry &r, const Transform &transform, Shader *shader, Mesh* mesh);

    void SnapToStart();

    void SetAutoCamera();
};


#endif //PLANETLANDING_RENDERER_H
