//
// Created by c3042750 on 25/10/2023.
//

#include "SceneNode.h"

SceneNode::SceneNode(Mesh *m, const Vector4& color) {
    Object = m;
    Color = color;
    Parent = nullptr;
    ModelScale = Vector3(1,1,1);
}

SceneNode::~SceneNode() {
    for (auto & i : children) {
        delete i;
    }
}

void SceneNode::AddChild(SceneNode *s) {
    children.push_back(s);
    s->Parent = this;
}

void SceneNode::Update(float dt) {
    if (Parent) {
        WorldPosition = Parent->WorldPosition * LocalPosition;
    } else {
        WorldPosition = LocalPosition;
    }

    for(auto i = children.begin(); i != children.end(); i++) {
        (*i)->Update(dt);
    }
}

void SceneNode::Draw(const OGLRenderer &r) {
    if (Object) Object->Draw();
}
