#pragma once

#include "DX12.h"

class DX12InputLayout
{
public:
	static const D3D12_INPUT_LAYOUT_DESC& getLayout(DX12InputLayoutType type);
protected:
	static D3D12_INPUT_LAYOUT_DESC layouts[4];
	static bool inited;

	DX12InputLayout() = default;
	static void init();
};