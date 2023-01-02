#pragma once
#include <dxgi.h>
#include "../Core/GraphicType.h"

int GetNumChannelsOfDXGIFormat(DXGI_FORMAT format);
DXGI_FORMAT getDXGIFormat(GPUBufferFormat format);