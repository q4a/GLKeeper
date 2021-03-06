#include "pch.h"
#include "GameObject.h"
#include "RenderableModel.h"
#include "SceneObject.h"
#include "ModelAssetsManager.h"

GameObject::GameObject(GameObjectID objectID, GameObjectDefinition* objectDefinition)
    : mID(objectID)
    , mDefinition(objectDefinition)
{
}

GameObject::~GameObject()
{
    FreeGameObjectEntity();
}

void GameObject::InitGameObjectEntity()
{


    // init physics component
}

void GameObject::FreeGameObjectEntity()
{

}

void GameObject::EnterGameWorld()
{
    InitGameObjectEntity();
}

void GameObject::LeaveGameWorld()
{
    FreeGameObjectEntity();
}

void GameObject::UpdateFrame()
{
}

bool GameObject::SetAnimationResource(const ArtResource& artResource)
{

    ModelAsset* modelAsset = nullptr;

    if (artResource.mResourceType == eArtResource_Mesh ||
        artResource.mResourceType == eArtResource_TerrainMesh ||
        artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        modelAsset = gModelsManager.LoadModelAsset(artResource.mResourceName);
    }

    debug_assert(modelAsset);

    //if (AnimatingMeshComponent* animMesh = mGameObjectEntity->GetComponent<AnimatingMeshComponent>())
    //{
    //    if (modelAsset == nullptr)
    //    {
    //        animMesh->SetModelAssetNull();
    //        return false;
    //    }

    //    animMesh->SetModelAsset(modelAsset);
    //    // setup animation
    //    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    //    {
    //        animMesh->ClearAnimation();
    //        animMesh->StartAnimation(artResource.mAnimationDesc.mFps, !artResource.mDoesntLoop);

    //        animParams.mFinalFrame = resource->mAnimationDesc.mFrames > 0 ? resource->mAnimationDesc.mFrames - 1 : 0;

    //        // set start / end animation mesh
    //        if (resource->mHasStartAnimation || resource->mHasEndAnimation)
    //        {
    //            if (resource->mHasStartAnimation)
    //            {
    //                animParams.mAnimMeshStart = gResources.GetMesh(resource->mResourceName + "start");
    //                debug_assert(animParams.mAnimMeshStart);
    //            }

    //            if (resource->mHasEndAnimation)
    //            {
    //                animParams.mAnimMeshEnd = gResources.GetMesh(resource->mResourceName + "end");
    //                debug_assert(animParams.mAnimMeshEnd);
    //            }
    //        }
    //        // allocate animator if it does not exists yet
    //        if (mMeshAnimation == nullptr)
    //        {
    //            mMeshAnimation = gGameScene.CreateAnimation(this);
    //        }
    //        mMeshAnimation->Setup(animParams);
    //        // set random progress
    //        if (resource->mRandomStartFrame)
    //        {
    //            mMeshAnimation->SetProgress(gGameScene.mRand.generate_float());
    //        }
    //    }
    //    return true;
    //}

    return false;
}
