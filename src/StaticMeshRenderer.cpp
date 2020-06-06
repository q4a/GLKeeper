#include "pch.h"
#include "StaticMeshRenderer.h"
#include "GpuBuffer.h"
#include "StaticMeshComponent.h"
#include "GraphicsDevice.h"
#include "RenderScene.h"

bool StaticMeshRenderer::Initialize()
{
    if (!mStaticMeshRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void StaticMeshRenderer::Deinit()
{
    mStaticMeshRenderProgram.FreeProgram();
}

void StaticMeshRenderer::Render(SceneRenderContext& renderContext, StaticMeshComponent* component)
{
    debug_assert(component);
    if (component->mDrawCalls.empty())
        return;

    mStaticMeshRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mStaticMeshRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndexBuffer);
    gGraphicsDevice.BindVertexBuffer(component->mVertexBuffer, Vertex3D_Format::Get());

    for (StaticMeshComponent::DrawCall& currDrawCall: component->mDrawCalls)
    {
        if (currDrawCall.mVertexCount == 0)
        {
            debug_assert(false);
            continue;
        }
        MeshMaterial* currMaterial = component->GetMeshMaterial(currDrawCall.mMaterialIndex);
        if (currMaterial == nullptr)
        {
            debug_assert(false);
            continue;
        }
        // filter out submeshes depending on current render pass
        if (renderContext.mCurrentPass == eRenderPass_Translucent && !currMaterial->IsTransparent())
            continue;

        if (renderContext.mCurrentPass == eRenderPass_Opaque && currMaterial->IsTransparent())
            continue;

        currMaterial->ActivateMaterial();
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            currDrawCall.mIndexDataOffset, 
            currDrawCall.mTriangleCount * 3);
    }
}

void StaticMeshRenderer::PrepareRenderdata(StaticMeshComponent* component)
{
    debug_assert(component);

    component->ClearDrawCalls();

    if (component->mTriMeshParts.empty())
        return;

    int overallVertexCount = 0;
    int overallTriangleCount = 0;
    for (StaticMeshComponent::TriMeshPart& currPart: component->mTriMeshParts)
    {
        overallVertexCount += (int) currPart.mVertices.size();
        overallTriangleCount += (int) currPart.mTriangles.size();
    }

    if (overallVertexCount == 0)
        return;

    int actualVBufferLength = overallVertexCount * Sizeof_Vertex3D;
    int actualIBufferLength = overallTriangleCount * sizeof(glm::ivec3);

    // allocate vertex buffer object
    if (component->mVertexBuffer == nullptr)
    {
        component->mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
        if (component->mVertexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // allocate index buffer object
    if (component->mIndexBuffer == nullptr)
    {
        component->mIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
        if (component->mIndexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    GpuBuffer* vertexBuffer = component->mVertexBuffer;
    GpuBuffer* indexBuffer = component->mIndexBuffer;

    // setup buffers
    if (!vertexBuffer->Setup(eBufferUsage_Dynamic, actualVBufferLength, nullptr) ||
        !indexBuffer->Setup(eBufferUsage_Dynamic, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }

    int numPieces = (int) component->mTriMeshParts.size();
    component->SetDrawCallsCount(numPieces);

    // upload parts
    unsigned char* vbufferPtr = vertexBuffer->LockData<unsigned char>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    unsigned char* ibufferPtr = indexBuffer->LockData<unsigned char>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    unsigned int iCurentPart = 0;
    unsigned int vertexDataOffset = 0;
    unsigned int triangleDataOffset = 0;

    for (StaticMeshComponent::TriMeshPart& currPart: component->mTriMeshParts)
    {
        StaticMeshComponent::DrawCall& drawCall = component->mDrawCalls[iCurentPart];
        drawCall.mMaterialIndex = iCurentPart;
        drawCall.mTriangleCount = (int) currPart.mTriangles.size();
        drawCall.mVertexCount = (int) currPart.mVertices.size();
        drawCall.mIndexDataOffset = triangleDataOffset;
        drawCall.mVertexDataOffset = vertexDataOffset;

        // copy geometry data
        unsigned int vertexDataLength = drawCall.mVertexCount * Sizeof_Vertex3D;
        unsigned int triangleDataLength = drawCall.mTriangleCount * sizeof(glm::ivec3);
        ::memcpy(vbufferPtr + vertexDataOffset, currPart.mVertices.data(), vertexDataLength);
        ::memcpy(ibufferPtr + triangleDataOffset, currPart.mTriangles.data(), triangleDataLength);

        vertexDataOffset += vertexDataLength;
        triangleDataOffset += triangleDataLength;
        ++iCurentPart;
    }

    if (!indexBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!vertexBuffer->Unlock())
    {
        debug_assert(false);
    }

    component->mRenderProgram = &mStaticMeshRenderProgram;
}

void StaticMeshRenderer::ReleaseRenderdata(StaticMeshComponent* component)
{
    debug_assert(component);
    component->mRenderProgram = nullptr;
    if (component->mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mVertexBuffer);
        component->mVertexBuffer = nullptr;
    }

    if (component->mIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mIndexBuffer);
        component->mIndexBuffer = nullptr;
    }
    component->ClearDrawCalls();
}