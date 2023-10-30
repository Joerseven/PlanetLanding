//
// Created by c3042750 on 26/10/2023.
//

#ifndef FIFTH_CUBEROBOT_H
#define FIFTH_CUBEROBOT_H
#include "SceneNode.h"

class CubeRobot: public SceneNode {
public:
    CubeRobot(Mesh* cube);
    ~CubeRobot(){};
    void Update(float dt) override;
protected:
    SceneNode* head;
    SceneNode* leftArm;
    SceneNode* rightArm;
};


#endif //FIFTH_CUBEROBOT_H
