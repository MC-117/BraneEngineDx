#include "IRendering.h"

bool RenderResource::isValid() const
{
    return instanceID >= 0 && instanceIDCount > 0 && material != NULL && meshPart != NULL && meshPart->isValid();
}
