#include "IMaterial.h"

IMaterial::IMaterial(MaterialDesc& desc) : desc(desc)
{
}

bool IMaterial::init()
{
    if (program)
        return program->init();
    return false;
}

bool IMaterial::isComputable() const
{
    if (program)
        return program->isComputable();
    return false;
}

void IMaterial::preprocess()
{
}

void IMaterial::processBaseData()
{
}

void IMaterial::processScalarData()
{
}

void IMaterial::processCountData()
{
}

void IMaterial::processColorData()
{
}

void IMaterial::processMatrixData()
{
}

void IMaterial::processTextureData()
{
}

void IMaterial::processImageData()
{
}

void IMaterial::postprocess()
{
}
