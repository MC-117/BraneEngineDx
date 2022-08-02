#pragma once

#include "UIWindow.h"

class ShaderManagerWindow : public UIWindow
{
public:
	ShaderManagerWindow(string name = "ShaderManager", bool defaultShow = false);
	virtual void onRenderWindow(GUIRenderInfo& info);
protected:
	static string getShaderFeatureName(Enum<ShaderFeature> feature);
	virtual void onShaderFileGUI(const ShaderFile& file);
	virtual void onShaderAdapterGUI(const ShaderAdapter& adapter);
	virtual void onShaderStageGUI(const ShaderStage& stage);
};