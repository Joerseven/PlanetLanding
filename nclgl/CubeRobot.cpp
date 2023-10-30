//
// Created by c3042750 on 26/10/2023.
//

#include "CubeRobot.h"

CubeRobot::CubeRobot(Mesh *cube) {
    auto *body = new SceneNode(cube, Vector4(1,0,0,1));
    body->ModelScale = Vector3(10,15,5);
    body->LocalPosition = Matrix4::Translation(Vector3(0,35,0));
    AddChild(body);

    head = new SceneNode(cube, Vector4(0,1,0,1));
    head->ModelScale = Vector3(5,5,5);
    head->LocalPosition = Matrix4::Translation(Vector3(0,30,0));
    body->AddChild(head);

    leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
    leftArm->ModelScale = Vector3(3,-18,3);
    leftArm->LocalPosition = Matrix4::Translation(Vector3(-12,30,-1));
    body->AddChild(leftArm);

    rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
    rightArm->ModelScale = Vector3(3,-18,3);
    rightArm->LocalPosition = Matrix4::Translation(Vector3(12,30,-1));
    body->AddChild(rightArm);

    auto leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
    leftLeg->ModelScale = Vector3(3,-17.5,3);
    leftLeg->LocalPosition = Matrix4::Translation(Vector3(-8,0,0));
    body->AddChild(leftLeg);

    auto rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
    rightLeg->ModelScale = Vector3(3,-17.5,3);
    rightLeg->LocalPosition = Matrix4::Translation(Vector3(8,0,0));
    body->AddChild(rightLeg);

}

void CubeRobot::Update(float dt) {
    LocalPosition = LocalPosition * Matrix4::Rotation(30.0f * dt, Vector3(0,1,0));
    head->LocalPosition = head->LocalPosition * Matrix4::Rotation(-30.0f * dt, Vector3(0,1,0));
    leftArm->LocalPosition = leftArm->LocalPosition * Matrix4::Rotation(-30.0f * dt, Vector3(1,0,0));
    rightArm->LocalPosition = rightArm->LocalPosition * Matrix4::Rotation(30.0f * dt, Vector3(1,0,0));
    SceneNode::Update(dt);
}
