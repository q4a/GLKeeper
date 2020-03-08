#include "pch.h"
#include "MeshViewGamestate.h"
#include "GameMain.h"
#include "GameScene.h"
#include "System.h"
#include "SceneObject.h"

#define MESH_VIEW_CAMERA_YAW_DEG    90.0f
#define MESH_VIEW_CAMERA_PITCH_DEG  75.0f
#define MESH_VIEW_CAMERA_DISTANCE   1.5f
#define MESH_VIEW_CAMERA_NEAR       0.1f
#define MESH_VIEW_CAMERA_FAR        10.0f
#define MESH_VIEW_CAMERA_FOVY       60.0f

void MeshViewGamestate::HandleGamestateEnter()
{
    mOrbitCameraControl.SetParams(MESH_VIEW_CAMERA_YAW_DEG, MESH_VIEW_CAMERA_PITCH_DEG, MESH_VIEW_CAMERA_DISTANCE);

    // setup scene camera
    gGameScene.mCamera.SetPerspective(gSystem.mSettings.mScreenAspectRatio, MESH_VIEW_CAMERA_FOVY, MESH_VIEW_CAMERA_NEAR, MESH_VIEW_CAMERA_FAR);
    gGameScene.mCamera.SetPosition(glm::vec3{0.0f, 1.0f, 2.0f});

    gGameScene.SetCameraControl(&mOrbitCameraControl);
    mOrbitCameraControl.ResetOrientation();

    cxx::aabbox aabox ( glm::vec3(-2.0f, -2.0f, -2.0f), glm::vec3(2.0f, 2.0f, 2.0f) );

    SceneObject* sceneObject = gGameScene.CreateSceneObject();
    sceneObject->SetLocalBoundingBox(aabox);
    gGameScene.AttachSceneObject(sceneObject);
}

void MeshViewGamestate::HandleGamestateLeave()
{
}

void MeshViewGamestate::HandleUpdateFrame()
{
}

void MeshViewGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}