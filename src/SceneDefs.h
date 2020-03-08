#pragma once

#include "ConsoleDefs.h"

// forwards
class SceneObject;
class SceneCamera;
class GameScene;
class SceneCameraControl;

namespace SceneAxes
{
    static const glm::vec3 X {1.0f, 0.0f, 0.0f};
    static const glm::vec3 Y {0.0f, 1.0f, 0.0f};
    static const glm::vec3 Z {0.0f, 0.0f, 1.0f};
};

enum eSceneCameraMode
{
    eSceneCameraMode_Perspective,
    eSceneCameraMode_Orthographic,
};

decl_enum_strings(eSceneCameraMode);

// scene cvars

extern CvarBoolean gCvarScene_DebugDrawAabb;
