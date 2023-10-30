//
// Created by c3042750 on 25/10/2023.
//

#ifndef FIFTH_SCENENODE_H
#define FIFTH_SCENENODE_H

#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "OGLRenderer.h"
#include <vector>

class SceneNode {
public:
    SceneNode(Mesh *m = nullptr, const Vector4& color = Vector4(1,1,1,1));
    ~SceneNode();

    void AddChild(SceneNode* s);
    virtual void Update(float dt);
    virtual void Draw(const OGLRenderer &r);

    std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); }
    std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.end(); }

    SceneNode* Parent;
    Mesh* Object;
    Matrix4 LocalPosition;
    Matrix4 WorldPosition;
    Vector3 ModelScale;
    Vector4 Color;



private:

    std::vector<SceneNode*> children;
};


#endif //FIFTH_SCENENODE_H
