#pragma once

#include "Texture2D.h"

class ENGINE_API Texture3D : public Texture
{
public:
    bool isStandard = false;

    Texture3D(const TextureInfo& info, bool isStandard = true);
    Texture3D(const string& file, bool isStandard = true);
    Texture3D(unsigned int width, unsigned int height, unsigned int depth, unsigned int channel, bool isStandard = false, const TextureInfo& info = TextureInfo());

    virtual ~Texture3D();

    void release();

    virtual bool isValid() const;
    virtual bool isStatic() const;

    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getChannel() const;
    virtual int getArrayCount() const;
    virtual int getMipLevels() const;
    virtual TexInternalType getFormat() const;

    virtual unsigned long long getTextureID();
    virtual ITexture* getVendorTexture() const;

    void setAutoGenMip(bool value);
    void setTextureInfo(const TextureInfo& info);

    virtual bool load(const string& file);
    virtual unsigned int bind();
    virtual unsigned int bindBase(unsigned int index);
    virtual unsigned int resize(unsigned int width, unsigned int height);
    virtual unsigned int resize(unsigned int width, unsigned int height, unsigned int depth);

    bool save(const string& file);

protected:
    bool readOnly = false;
    TextureDesc desc;
    ITexture3D* vendorTexture = NULL;
    static bool isLoadDefaultTexture;

    void newVendorTexture();
};
