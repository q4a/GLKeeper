#pragma once

#include "Texture2D_Image.h"

// texture 2d resource
class Texture2D: public cxx::noncopyable
{
public:
    // readonly
    std::string mTextureName;
    Texture2D_Desc mTextureDesc;

    Texture2D* mProxyTexture = nullptr;

public:
    Texture2D(const std::string& textureName);
    ~Texture2D();

    // load texture data from file mTextureName and upload it to gpu
    // @returns false on error
    bool LoadTexture();
    void FreeTexture();

    // create texture from source image and upload it to gpu
    // @param sourceImage: Image data
    // @returns false on error
    bool CreateTexture(const Texture2D_Image& sourceImage);
    bool CreateTexture(const Texture2D_Desc& sourceDesc, const void* textureData);

    // update image, does not work with proxy texture
    bool UpdateTexture(int mipmap, const Rectangle& textureRect, const void* textureData);
    bool UpdateTexture(int mipmap, const void* textureData);

    void SetProxyTexture(Texture2D* proxyTexture);

    // set texture filtering and repeating modes
    // @param samplerState: Sampler state params
    void SetSamplerState(const TextureSamplerState& samplerState);

    // bind texture to specific texture unit
    // will load texture if it not loaded yet
    // @param textureUnit: Texture unit identifier
    // @returns false on error
    bool ActivateTexture(eTextureUnit textureUnit);

    // test whether texture is initialized and currently active
    bool IsTextureLoaded() const;
    bool IsTextureActivate(eTextureUnit textureUnit) const;
    bool IsTextureActivate() const;

    // set texture persistent
    void SetPersistent(bool isPersistent);

    bool IsLoadedFromFile() const { return mLoadedFromFile; }
    bool IsPersistent() const { return mPersistent; }

    bool HasProxyTexture() const;

private:
    // internals
    void DestroyTextureObject();

private:
    bool mLoadedFromFile = false; // image was loaded from image file so it can be reloaded
    bool mPersistent = false; // once it was loaded keep alive forever

    TextureSamplerState mSamplerState;
    GpuTexture2D* mGpuTextureObject = nullptr;
};