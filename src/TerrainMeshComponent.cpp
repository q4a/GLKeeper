#include "pch.h"
#include "TerrainMeshComponent.h"
#include "GameObject.h"
#include "GraphicsDevice.h"
#include "TerrainTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"
#include "TransformComponent.h"
#include "TerrainMeshRenderer.h"
#include "RenderManager.h"

TerrainMeshComponent::TerrainMeshComponent(GameObject* gameObject) 
    : RenderableComponent(gameObject)
    , mMeshInvalidated()
{
    debug_assert(mGameObject);
    mGameObject->mDebugColor = Color32_Brown;
}

void TerrainMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    if (IsMeshInvalidated())
    {
        PrepareRenderResources();
    }
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.Render(renderContext, this);
}

void TerrainMeshComponent::SetTerrainArea(const Rectangle& mapArea)
{
    if (mMapTerrainRect == mapArea)
        return;

    mMapTerrainRect = mapArea;

    cxx::aabbox sectorBox;
    // min
    sectorBox.mMin.x = (mMapTerrainRect.x * TERRAIN_BLOCK_SIZE) - TERRAIN_BLOCK_HALF_SIZE;
    sectorBox.mMin.y = 0.0f;
    sectorBox.mMin.z = (mMapTerrainRect.y * TERRAIN_BLOCK_SIZE) - TERRAIN_BLOCK_HALF_SIZE;
    // max
    sectorBox.mMax.x = sectorBox.mMin.x + (mMapTerrainRect.w * TERRAIN_BLOCK_SIZE);
    sectorBox.mMax.y = 3.0f;
    sectorBox.mMax.z = sectorBox.mMin.z + (mMapTerrainRect.h * TERRAIN_BLOCK_SIZE);

    TransformComponent* transformComponent = mGameObject->mTransformComponent;
    transformComponent->SetLocalBoundingBox(sectorBox);

    InvalidateMesh();
}

void TerrainMeshComponent::InvalidateMesh()
{
    mMeshInvalidated = true;
}

bool TerrainMeshComponent::IsMeshInvalidated() const
{
    return mMeshInvalidated;
}

void TerrainMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    mMeshInvalidated = false;

    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void TerrainMeshComponent::ReleaseRenderResources()
{
    TerrainMeshRenderer& renderer = gRenderManager.mTerrainMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}