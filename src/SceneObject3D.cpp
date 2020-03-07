#include "pch.h"
#include "SceneObject3D.h"
#include "GameScene.h"

SceneObject3D::SceneObject3D()
    : mListNodeTransformed(this)
    , mListNodeAttached(this)
    , mTransformationDirty()
    , mBoundingBoxDirty()
    , mTransformation(1.0f)
    , mScaling(1.0f)
    , mPosition()
    , mDirectionRight(SceneAxes::X)
    , mDirectionUpward(SceneAxes::Y)
    , mDirectionForward(SceneAxes::Z)
{
}

SceneObject3D::~SceneObject3D()
{
    // entity must be detached from scene before destroy
    debug_assert(IsAttachedToScene() == false);
}

void SceneObject3D::ComputeTransformation()
{
    // refresh transformations matrix
    if (mTransformationDirty)
    {
        glm::mat4 orientation {1.0f};
        orientation[0] = glm::vec4(mDirectionRight, 0);
        orientation[1] = glm::vec4(mDirectionUpward, 0);
        orientation[2] = glm::vec4(mDirectionForward, 0);

        glm::vec3 scaling {mScaling, mScaling, mScaling};
        mTransformation = glm::translate(mPosition) * orientation * glm::scale(scaling);
        mTransformationDirty = false;
        // force refresh world space bounds
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
        return;
    }

    // refresh world space bounding box
    if (mBoundingBoxDirty)
    {
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
    }
}

void SceneObject3D::SetLocalBoundingBox(const cxx::aabbox& aabox)
{
    mBounds = aabox;

    InvalidateBounds();
}

void SceneObject3D::SetPositionOnScene(const glm::vec3& position)
{
    mPosition = position;

    InvalidateTransform();
}

void SceneObject3D::SetScaling(float scaling)
{
    mScaling = scaling;

    InvalidateTransform();
}

void SceneObject3D::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mDirectionForward = directionForward;
    mDirectionRight = directionRight;
    mDirectionUpward = directionUpward;

    InvalidateTransform();
}

void SceneObject3D::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    debug_assert(!"not yet implemented"); // todo
}

void SceneObject3D::OrientTowards(const glm::vec3& point)
{
    OrientTowards(point, SceneAxes::Y);
}

void SceneObject3D::Rotate(const glm::vec3& rotationAxis, float rotationAngle)
{
    glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle, rotationAxis)); 

    mDirectionForward = glm::normalize(rotationMatrix * mDirectionForward);
    mDirectionRight = glm::normalize(rotationMatrix * mDirectionRight);
    mDirectionUpward = glm::normalize(rotationMatrix * mDirectionUpward);

    InvalidateTransform();
}

void SceneObject3D::Translate(const glm::vec3& translation)
{
    mPosition += translation;

    InvalidateTransform();
}

void SceneObject3D::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    glm::vec3 zaxis = glm::normalize(point - mPosition);
    glm::vec3 xaxis = glm::normalize(glm::cross(upward, zaxis));
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // now assign orientation vectors
    SetOrientation(xaxis, zaxis, yaxis);
}

void SceneObject3D::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScaling = 1.0f;
    mPosition = glm::vec3{0.0f};
    mDirectionRight = SceneAxes::X;
    mDirectionUpward = SceneAxes::Y;
    mDirectionForward = SceneAxes::Z;

    InvalidateTransform();
}

void SceneObject3D::InvalidateTransform()
{
    if (mTransformationDirty)
        return;

    mTransformationDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 

    if (IsAttachedToScene())
    {
        gGameScene.HandleSceneObjectTransformChange(this);
    }
}

void SceneObject3D::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    if (IsAttachedToScene())
    {
        gGameScene.HandleSceneObjectTransformChange(this);
    }
}

void SceneObject3D::ResetOrientation()
{
    SetOrientation(SceneAxes::X, SceneAxes::Z, SceneAxes::Y);
}

bool SceneObject3D::IsAttachedToScene() const
{
    return mListNodeAttached.is_linked();
}

